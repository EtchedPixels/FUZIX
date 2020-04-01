#include <kernel.h>
#include <tty.h>
#include <vt.h>

#ifdef CONFIG_VT


#include <devtty.h>
/*
 *	Mini vt52 terminal emulation
 *
 *	The caller is required to provide
 *
 *	cursor_off();
 *	cursor_disable();
 *	cursor_on(newy, newx)
 *	clear_across(y,x,num)
 *	clear_lines(y,num)
 *	scroll_up();
 *	scroll_down();
 *	plot_char(y, x, c);
 *
 *	VT_RIGHT, VT_BOTTOM for the size.
 *
 *	For now we don't do region scrolling, status line stuff and the like
 *	that some vt52 alikes have. If your video memory is banked then
 *	you probably want to put your helpers in common, but there should
 *	be no need for any of this logic to be there
 *
 *	For the simple case of a vt permanently mapped into the kernel space
 *	and using character mode the driver can provide all the other logic
 *	for free.
 *	
 *	In that case
 *	define CONFIG_VT_SIMPLE
 *	VT_BASE is the base address in kernelspace
 *	VT_WIDTH is the line width (including padding)
 *
 *	This code can be banked on its own. If you touch it on Z80 make
 *	very sure you inspect the asm output for calls to compiler helpers
 *	and don't add any.
 *
 *	Extensions in use
 *	- Esc a c	Set vtattr bits (inverse, etc)
 *
 *	- Esc b c	Set ink colour
 *	- Esc c c	Set paper colour
 *	Where c uses the low 4bits to provide IBRG colours
 *		0 black
 *		1 blue
 *		2 red
 *		3 magenta or similar
 *		4 green
 *		5 cyan
 *		6 yellow
 *		7 white
 *		8-15 brighter versions - if available
 *	Bit 4 is reserved (must be 0 if bit 5 is 0)
 *	Bit 5 indicates bit 4-0 are platform specific colour
 *	Bit 6 should be set to keep it in the ascii range
 *	Bit 7 should be clear
 *
 *	Possible VT extensions to look at
 *	- Esc-L		Insert blank line, move lines below down
 *	- ESC-M		Delete cursor line, move up blank bottom
 *	- ESC-d		erase start to cursor inclusive
 *	- ESC-j		save cursor y/x
 *	- ESC-k		restore cursor
 *	- ESC-l		erase line, cursor to left
 *	- ESC-o		erase from start of line to cursor (inclusive)
 *
 *	- Some way to set border colour on consoles.
 */


static uint8_t vtmode;
uint8_t vtattr;
uint8_t vtink = 7;
uint8_t vtpaper;
static signed char cursorx;
static signed char cursory = VT_INITIAL_LINE;
static signed char ncursory;
static uint8_t cursorhide;
static uint8_t vtpend;
static uint8_t vtbusy;

static void cursor_fix(void)
{
	if (cursorx < 0) {
		cursorx = VT_RIGHT;
		cursory--;
	}
	if (cursory < 0)
		cursory = 0;
	if (cursorx > VT_RIGHT) {
		cursorx = 0;
		cursory++;
	}
	if (cursory > VT_BOTTOM) {
		scroll_up();
		clear_lines(VT_BOTTOM, 1);
		cursory--;
	}
}

static void charout(unsigned char c)
{
	/* Fast path printable symbols */
	if (c <= 0x1b) {
		if (c == 7) {
			do_beep();
			return;
		}
		if (c == 8) {
			cursorx--;
			goto fix;
		}
		if (c == 9) {
			do {
				charout(' ');
			} while (cursorx&7);
			goto fix;
		}
		if (c == 10) {
			cursory++;
			goto fix;
		}
		if (c == 13) {
			cursorx = 0;
			return;
		}
		if (c == 0x1b) {
			vtmode = 1;
			return;
		}
	}
	plot_char(cursory, cursorx, c);
	cursorx++;
fix:
	cursor_fix();
}


static int escout(unsigned char c)
{
	if (c == 'A') {
		if (cursory)
			cursory--;
		return 0;
	}
	if (c == 'B') {
		if (cursory < VT_BOTTOM)
			cursory++;
		return 0;
	}
	if (c == 'C') {
		if (cursorx < VT_RIGHT)
			cursorx++;
		return 0;
	}
	if (c == 'D') {
		if (cursorx)
			cursorx--;
		return 0;
	}
	if (c == 'E') {
		clear_lines(0, VT_BOTTOM + 1);
		return 0;
	}
	if (c == 'H') {
		cursorx = 0;
		cursory = 0;
		return 0;
	}
	if (c == 'I') {
		if (cursory)
			cursory--;
		else {
			scroll_down();
			clear_lines(0, 1);
		}
		return 0;
	}
	if (c == 'J') {
		clear_across(cursory, cursorx, VT_RIGHT - cursorx + 1);
		clear_lines(cursory + 1, VT_BOTTOM - cursory);
		return 0;
	}
	if (c == 'K') {
		clear_across(cursory, cursorx, VT_RIGHT - cursorx + 1);
		return 0;
	}
	if (c == 'e') {
		cursorhide = 0;
		return 0;
	}
	if (c == 'f') {
		cursorhide = 1;
		cursor_disable();
		return 0;
	}
	if (c == 'Y')
		return 2;
	if (c == 'a')
		return 4;
	if (c == 'b')
		return 5;
	if (c == 'c')
		return 6;
	return 0;
}

void vt_cursor_off(void)
{
	if (!cursorhide)
		cursor_off();
}

void vt_cursor_on(void)
{
	if (!cursorhide)
		cursor_on(cursory, cursorx);
}

/* VT52 alike functionality */
void vtoutput(unsigned char *p, unsigned int len)
{
	irqflags_t irq;
	uint8_t cq;

	/* We can get re-entry into the vt code from tty echo. This is one of
	   the few places in Fuzix interrupts bite us this way.

	   If we have a clash then we queue the echoed symbol and print it
	   in the thread of execution it interrupted. We only queue one so
	   in theory might lose the odd echo - but the same occurs with real
	   uarts. If anyone actually has printing code slow enough this is a
	   problem then vtpend can turn into a small queue */
	irq = di();
	if (vtbusy) {
		vtpend = *p;
		irqrestore(irq);
		return;
	}
	vtbusy = 1;
	irqrestore(irq);
	vt_cursor_off();
	/* FIXME: do we ever get called with len > 1, if not we could strip
	   this right down */
	do {
		while (len--) {
			unsigned char c = *p++;
			if (vtmode == 0) {
				charout(c);
				continue;
			}
			if (vtmode == 1) {
				vtmode = escout(c);
				continue;
			}
			if (vtmode == 2) {
				ncursory = c - ' ';
				vtmode++;
				continue;
			} else if (vtmode == 3) {
				int ncursorx = c - ' ';
				if (ncursory >= 0 && ncursory <= VT_BOTTOM)
					cursory = ncursory;
				if (ncursorx >= 0 && ncursorx <= VT_RIGHT)
					cursorx = ncursorx;
				vtmode = 0;
			} else if (vtmode == 4 ){
				vtattr = c;
				vtmode = 0;
				vtattr_notify();
				continue;
			} else if (vtmode == 5) {
				vtink = c;
				vtmode = 0;
				vtattr_notify();
				continue;
			} else if (vtmode == 6) {
				vtpaper = c;
				vtmode = 0;
				vtattr_notify();
				continue;
			}
		}
		/* Copy the pending symbol and clear the buffer */
		cq = vtpend;
		vtpend = 0;
		/* Any loops print the single byte in cq */
		p = &cq;
		len = 1;
		/* Until we don't get interrupted */
	} while(cq);
	vt_cursor_on();
	vtbusy = 0;
}

/* Note: multiple vt switching handled by platform wrapper */
int vt_ioctl(uint_fast8_t minor, uarg_t request, char *data)
{
	if (minor <= MAX_VT) {
		switch(request) {
#ifdef KEY_ROWS
			case KBMAPSIZE:
				return KEY_ROWS << 8 | KEY_COLS;
			case KBMAPGET:
				return uput(keymap, data, sizeof(keymap));
			case KBSETTRANS:
				if (uget(data, keyboard, sizeof(keyboard)) == -1)
					return -1;
				return uget(data + sizeof(keyboard),
					shiftkeyboard,
					sizeof(shiftkeyboard));
			case KBRATE:
				if (uget(data, &keyrepeat, sizeof(keyrepeat)) == -1)
					return -1;
				keyrepeat.first *= (TICKSPERSEC/10);
				keyrepeat.continual *= (TICKSPERSEC/10);
				return 0;
#endif					
			case VTSIZE:
				return (VT_BOTTOM + 1) << 8 | (VT_RIGHT + 1);
			case VTATTRS:
				return vtattr_cap;
		}
	}
	return tty_ioctl(minor, request, data);
}

int vt_inproc(uint_fast8_t minor, uint_fast8_t c)
{
#ifdef CONFIG_UNIKEY
	if (c == KEY_POUND) {
		tty_inproc(minor, 0xC2);
		return tty_inproc(minor, 0xA3);
	}
	if (c == KEY_HALF) {
		tty_inproc(minor, 0xC2);
		return tty_inproc(minor, 0xBD);
	}
	if (c == KEY_DOT) {
		tty_inproc(minor, 0xC2);
		return tty_inproc(minor, 0xB7);
	}
	if (c == KEY_EURO) {
		tty_inproc(minor, 0xE2);
		tty_inproc(minor, 0x82);
		return tty_inproc(minor, 0xAC);
	}
        if (c == KEY_YEN) {
		tty_inproc(minor, 0xC2);
		return tty_inproc(minor, 0xA5);
	}
	if (c == KEY_COPYRIGHT) {
		tty_inproc(minor,0xC2);
		return tty_inproc(minor, 0xA9);
	}
#endif
	if (c > 0x9F) {
		tty_inproc(minor, KEY_ESC);
		c &= 0x7F;
	}
	return tty_inproc(minor, c);
}

void vtinit(void)
{
	vtmode = 0;
	vtattr_notify();
	clear_lines(0, VT_BOTTOM + 1);
	cursor_on(0, 0);
}

#ifdef CONFIG_VT_MULTI

void vt_save(struct vt_switch *vt)
{
	vt->vtmode = vtmode;
	vt->vtattr = vtattr;
	vt->cursorx = cursorx;
	vt->cursory = cursory;
	vt->ncursory = ncursory;
	vt->cursorhide = cursorhide;
	vt->ink = vtink;
	vt->paper = vtpaper;
}

void vt_load(struct vt_switch *vt)
{
	vtmode = vt->vtmode;
	vtattr = vt->vtattr;
	cursorx = vt->cursorx;
	cursory = vt->cursory;
	ncursory = vt->ncursory;
	cursorhide = vt->cursorhide;
	vtink = vt->ink;
        vtpaper = vt->paper;
	vtattr_notify();
}
#endif

#ifdef CONFIG_VT_SIMPLE


static unsigned char *cpos;
static unsigned char csave;

static uint8_t *char_addr(unsigned int y1, unsigned char x1)
{
	/* See SDCC bug #2332 */
	return VT_BASE + VT_WIDTH * y1 + (uint16_t)x1;
}

void cursor_off(void)
{
	if (cpos)
		*cpos = csave;
}

/* Only needed for hardware cursors */
void cursor_disable(void)
{
}

void cursor_on(int8_t y, int8_t x)
{
	cpos = char_addr(y, x);
	csave = *cpos;
	*cpos = VT_MAP_CHAR('_');
}

void plot_char(int8_t y, int8_t x, uint16_t c)
{
	*char_addr(y, x) = VT_MAP_CHAR(c);
}

void clear_lines(int8_t y, int8_t ct)
{
	unsigned char *s = char_addr(y, 0);
	memset(s, ' ', ct * VT_WIDTH);
}

void clear_across(int8_t y, int8_t x, int16_t l)
{
	unsigned char *s = char_addr(y, x);
	memset(s, ' ', l);
}

void vtattr_notify(void)
{
}

void scroll_up(void)
{
	memmove(VT_BASE, VT_BASE + VT_WIDTH, VT_WIDTH * VT_BOTTOM);
}

void scroll_down(void)
{
	memmove(VT_BASE + VT_WIDTH, VT_BASE, VT_WIDTH * VT_BOTTOM);
}

#endif
#endif
