#include <kernel.h>
#include <tty.h>
#include <vt.h>
#include <devtty.h>

#ifdef CONFIG_VT
/*
 *	Mini vt52 terminal emulation
 *
 *	The caller is required to provide
 *
 *	cursor_off();
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
 */


static uint8_t vtmode;
static signed char cursorx;
static signed char cursory = VT_INITIAL_LINE;
static signed char ncursory;

static void cursor_fix(void)
{
	if (cursorx < 0) {
		cursorx = 0;
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
		} while (cursorx%8);
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
		clear_lines(0, VT_HEIGHT);
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
		clear_across(cursory, cursorx, VT_RIGHT - cursorx);
		clear_lines(cursory + 1, VT_BOTTOM - cursory);
		return 0;
	}
	if (c == 'K') {
		clear_across(cursory, cursorx, VT_RIGHT - cursorx);
		return 0;
	}
	if (c == 'Y')
		return 2;
	return 0;
}


/* VT52 alike functionality */
void vtoutput(unsigned char *p, unsigned int len)
{
	cursor_off();
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
		} else {
			int ncursorx = c - ' ';
			if (ncursory >= 0 && ncursorx <= VT_BOTTOM)
				cursory = ncursory;
			if (ncursorx >= 0 && ncursorx <= VT_RIGHT)
				cursorx = ncursorx;
		}
	}
	cursor_on(cursory, cursorx);
}

int vt_ioctl(uint8_t minor, uint16_t request, char *data)
{
	/* FIXME: need to address the multiple vt switching case
	   here.. probably need to switch vt */
	if (minor <= MAX_VT) {
		switch(request) {
			case KBMAPSIZE:
				return KEY_ROWS << 8 | KEY_COLS;
			case KBMAPGET:
				return uput(keymap, data, sizeof(keymap));
			case KBSETTRANS:
				if (esuper())
					return -1;
				if (uget(keyboard, data, sizeof(keyboard)) == -1)
					return -1;
				return uget(shiftkeyboard,
					data + sizeof(keyboard),
					sizeof(shiftkeyboard));
			case VTSIZE:
				return VT_HEIGHT << 8 | VT_WIDTH;
		}
	}
	return tty_ioctl(minor, request, data);
}

int vt_inproc(uint8_t minor, unsigned char c)
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
	clear_lines(0, VT_HEIGHT);
	cursor_on(0, 0);
}

#ifdef CONFIG_VT_MULTI

void vt_save(struct vt_switch *vt)
{
	vt->vtmode = vtmode;
	vt->cursorx = cursorx;
	vt->cursory = cursory;
	vt->ncursory = ncursory;
}

void vt_load(struct vt_switch *vt)
{
	vtmode = vt->vtmode;
	cursorx = vt->cursorx;
	cursory = vt->cursory;
	ncursory = vt->ncursory;
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

/* FIXME: these should use memmove */

void scroll_up(void)
{
	memcpy(VT_BASE, VT_BASE + VT_WIDTH, VT_WIDTH * VT_BOTTOM);
}

void scroll_down(void)
{
	memcpy(VT_BASE + VT_WIDTH, VT_BASE, VT_WIDTH * VT_BOTTOM);
}

#endif
#endif
