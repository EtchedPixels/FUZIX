#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <tty.h>
#include <vt.h>
#include <devtty.h>
#include <stdarg.h>
#include <nascom.h>

static char tbuf1[TTYSIZ];
static char tbuf2[TTYSIZ];

struct vt_repeat keyrepeat;

__sfr __at 0x01 s6402_data;
__sfr __at 0x02 s6402_status;

struct s_queue ttyinq[NUM_DEV_TTY + 1] = {	/* ttyinq[0] is never used */
	{NULL, NULL, NULL, 0, 0, 0},
	{tbuf1, tbuf1, tbuf1, TTYSIZ, 0, TTYSIZ / 2},
	{tbuf2, tbuf2, tbuf2, TTYSIZ, 0, TTYSIZ / 2},
};

tcflag_t termios_mask[NUM_DEV_TTY + 1] = {
	0,
	_CSYS,
	_CSYS
};

/* Write to system console */
void kputchar(char c)
{
	if (c == '\n')
		tty_putc(1, '\r');
	tty_putc(1, c);
}

ttyready_t tty_writeready(uint8_t minor)
{
	uint8_t reg;
	if (minor == 1)
		return TTY_READY_NOW;
	reg = s6402_status;
	return (reg & 0x40) ? TTY_READY_NOW : TTY_READY_SOON;
}

void tty_putc(uint8_t minor, unsigned char c)
{
	if (minor == 2)
		s6402_data = c;
	else
		vtoutput(&c, 1);
}

void tty_poll(void)
{
	uint8_t reg = s6402_status;
	if (reg & 0x80) {
		reg = s6402_data;
		tty_inproc(2, reg);
	}
}

void tty_setup(uint8_t minor, uint8_t flags)
{
	/* The console is a crt/keyboard, the 6402 is set by jumpers */
}

int tty_carrier(uint8_t minor)
{
	return 1;
}

void tty_sleeping(uint8_t minor)
{
	used(minor);
}

void tty_data_consumed(uint8_t minor)
{
	used(minor);
}

uint8_t keymap[9];
static uint8_t keyin[9];
static uint8_t keybyte, keybit;
static uint8_t newkey;
static int keysdown = 0;
static uint8_t shiftmask[9] = {
	0, 0, 0, 0, 0, 0, 0, 0, 24
};

__sfr __at 0 kbd_data;

static void keyproc(void)
{
	int i;
	uint8_t key;

	kbd_data = 2;		/* Reset the keyboard */

	for (i = 0; i < 9; i++) {
		/* Read the keyboard row */
		keyin[i] = ~kbd_data & 0x7F;
		kbd_data = 1;	/* Clock the keyboard */
		key = keyin[i] ^ keymap[i];
		if (key) {
			uint8_t n;
			uint8_t m = 1;
			for (n = 0; n < 8; n++) {
				if ((key & m) && (keymap[i] & m)) {
					if (!(shiftmask[i] & m))
						keysdown--;
				}
				else if ((key & m) && !(keymap[i] & m)) {
					if (!(shiftmask[i] & m)) {
						keysdown++;
						newkey = 1;
						keybyte = i;
						keybit = n;
					}
				}
				m += m;

			}
		}
		keymap[i] = keyin[i];
	}
}

/*
 *	The Nascom has a surprisingly complete and good keyboard. It's lacking
 *	only a capslock key and {|}. There are some oddities however
 *
 *	Escape is shift-enter, cs is shift-backspace, there is a lf/ch button
 *	and a graph button that's basically unused but will no doubt excite
 *	emacs people 8)
 *
 *	The keyboard bits are rather confusing as they are not wired 1:1 as you
 *	might expect. Instead the order is
 *
 *	S1 S2 S0 S4 S5 S3 S6 *
 */

uint8_t keyboard[9][8] = {
	/* just a shift in the top row */
	{0, 0, 0, 0, 0, 0, 0, 0},
	{ 'h', 'b', '5', 'f', 'x', 't', KEY_UP, },
	{ 'j', 'n', '6', 'd', 'z', 'y', KEY_LEFT, },
	{ 'k', 'm', '7', 'e', 's', 'u', KEY_DOWN, },
	{ 'l', ',', '8', 'w', 'a', 'i', KEY_RIGHT, },
	/*  FIXME: the ? is the graph key - what to do with it */
	{ ';', '.', '9', '3', 'q', 'o', '?', },
	{ ':', '/', '0', '2', '1', 'p', '[', },
	{ 'g', 'v', '4', 'c', ' ', 'r', ']', },
	/* What to do with ch ? */
	{ KEY_BS, KEY_ENTER, '-', 0, 0, '@', '?', },
	/* Ch, @ shift cntrl - ... */
};

uint8_t shiftkeyboard[9][8] = {
	/* just a shift in the top row */
	{0, 0, 0, 0, 0, 0, 0, 0},
	{ 'H', 'B', '%', 'F', 'X', 'T', KEY_UP, },
	{ 'J', 'N', '&', 'D', 'Z', 'Y', KEY_LEFT, },
	{ 'K', 'M', '\'', 'E', 'S','U', KEY_DOWN, },
	{ 'L', '<', '(', 'W', 'A', 'I', KEY_RIGHT, },
	/* FIXME key graph ? */
	{ '+', '>', ')', KEY_POUND, 'Q', 'O', '?', },
	{ '*', '/', '"', '!', 'P', '\\', ':', },
	{ 'G', 'V', '$', 'C', ' ', 'R', '_',  },
	/* What to do with ch ? */
	{ KEY_BS, KEY_ESC, '=', 0, 0, '@', '?', },
	/* Ch, @ shift cntrl - ... */
};

static uint8_t kbd_timer;

static void keydecode(void)
{
	uint8_t c;
	uint8_t m = 0;
	uint8_t shift = 0;

	if ((keymap[8] & 0x20) || (keymap[0] & 0x10)) {	/* shift */
		c = shiftkeyboard[keybyte][keybit];
		shift = 1;
	} else
		c = keyboard[keybyte][keybit];

	if (keymap[8] & 0x10) {	/* control */
		if (shift) {	/* shift */
			if (c == '(')
				c = '{';
			if (c == ')')
				c = '}';
			if (c == '^')
				c = '|';
		} else if (c > 31 && c < 127)
			c &= 31;
	}
	if (c)
		vt_inproc(1, c);
}

void kbd_poll(void)
{
	newkey = 0;
	keyproc();
	if (keysdown && keysdown < 3) {
		if (newkey) {
			keydecode();
			kbd_timer = keyrepeat.first;
		} else if (!--kbd_timer) {
			keydecode();
			kbd_timer = keyrepeat.continual;
		}
	}
}
