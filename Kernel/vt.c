#include <kernel.h>
#include <vt.h>

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


static int vtmode;
static signed char cursorx;
static signed char cursory;
static int ncursory;

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

void vtinit(void)
{
	vtmode = 0;
	clear_lines(0, VT_HEIGHT);
	cursor_on(0, 0);
}


#ifdef CONFIG_VT_SIMPLE

static unsigned char *cpos;
static unsigned char csave;

static uint8_t *char_addr(unsigned int y1, unsigned char x1)
{
	return VT_BASE + VT_WIDTH * y1 + x1;
}

void cursor_off(void)
{
	*cpos = csave;
}

void cursor_on(int8_t y, int8_t x)
{
	cpos = char_addr(y, x);
	csave = *cpos;
	*cpos = '_';
}

void plot_char(int8_t y, int8_t x, uint16_t c)
{
	*char_addr(y, x) = c;
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
