/*
 *	The MicroBee has a console interface which we map as tty1. It's a
 *	rather normal 6845 setup but with options to do character based high
 *	res with 8x16 patterns, two colours per pattern.
 *
 *	The input side on the other hand is a bit different. The TC has a
 *	rather normal sane interface but the older machines wire the keyboard
 *	through some of the address scanning of the 6845 and the light pen
 *	input so that the raster scan does a (slow) keyboard scan. Sucky but
 *	cheap.
 *
 *	The serial ports are wired to a pair of PIO controllers not to an
 *	actual UART although there are interrupting on the edges. We don't
 *	support those for now. Some machines had add in Z8530 based
 *	interfaces at 0x68/0x69:  we need to look at those some day.
 */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <tty.h>
#include <vt.h>
#include <devtty.h>
#include <stdarg.h>

static char tbuf1[TTYSIZ];

uint8_t vtattr_cap;

struct s_queue ttyinq[NUM_DEV_TTY + 1] = {	/* ttyinq[0] is never used */
	{NULL, NULL, NULL, 0, 0, 0},
	{tbuf1, tbuf1, tbuf1, TTYSIZ, 0, TTYSIZ / 2},
};

/* Write to system console */
void kputchar(char c)
{
	if (c == '\n')
		tty_putc(1, '\r');
	tty_putc(1, c);
}

uint8_t tty_writeready(uint8_t minor)
{
	minor;
	return TTY_READY_NOW;
}

void tty_putc(uint8_t minor, unsigned char c)
{
	minor;
	vtoutput(&c, 1);
}

void tty_setup(uint8_t minor)
{
	minor;
}

void tty_sleeping(uint8_t minor)
{
	minor;
}

int tty_carrier(uint8_t minor)
{
        minor;
	return 1;
}

#if 0
static uint8_t shiftmask[8] = {
	0, 0, 0, 0, 0, 0, 0, 7
};

static void keyproc(void)
{
	int i;
	uint8_t key;

	for (i = 0; i < 8; i++) {
		/* Set one of A0 to A7, and read the byte we get back.
		   Invert that to get a mask of pressed buttons */
		keyin[i] = *(uint8_t *) (0xF400 | (1 << i));
		key = keyin[i] ^ keymap[i];
		if (key) {
			int n;
			int m = 1;
			for (n = 0; n < 8; n++) {
				if ((key & m) && (keymap[i] & m)) {
					if (!(shiftmask[i] & m))
						keysdown--;
				}
				if ((key & m) && !(keymap[i] & m)) {
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

#endif

static uint8_t keymap[15];
static uint8_t keyin[15];
static uint8_t keybyte, keybit;
static uint8_t newkey;
static int keysdown = 0;

/* These are not completely correct for the TC yet */
static uint8_t keyboard_tc[15][8] = {
	/* 0x */
	{ KEY_F1, KEY_ESC, KEY_TAB, KEY_STOP, 0, '0','.',' ' },
	{ KEY_F2, '1', 'q', 'a', 0, 0/*Capslock*/, CTRL('M'), KEY_INSERT },
	/* 1x */
	{ KEY_F3, '2', 'w', 's', '+', '2', '3', 'z' },
	{ KEY_F4, '3', 'e', 'd', '-', '5', '6', 'x' },
	/* 2x */
	{ KEY_F5, '4', 'r', 'f', '*', '8', '9', 'c' },
	{ KEY_F6, '5', 't', 'g', '7', '1', '4', 'v' },
	/* 3x */
	{ KEY_F7, '6', 'y', 'h', '/', KEY_DOWN, KEY_RIGHT, 'b' },
	{ KEY_F8, '7', 'u', 'j',  0 , KEY_LEFT,  0 , 'n' },
	/* 4x */
	{ KEY_F9, '8', 'i', 'k',  0 ,  0 , KEY_UP, 'm' },
	{ KEY_F10, '9', 'o', 'l', 0, KEY_BS, KEY_ENTER, ',' },
	/* 5x */
	{ KEY_F11, '0', 'p', ';', KEY_DEL, '`', '\\', '.' },
	{ KEY_F12, '-', '[', '\'', 0, '=', ']', '/' },
	/* 60 shift 67 ctrl 70 alt */
};

static uint8_t shift_keyboard_tc[15][8] = {
	/* 0x */
	{ KEY_F1, KEY_ESC, KEY_TAB, KEY_STOP, 0, '0','.',' ' },
	{ KEY_F2, '!', 'Q', 'A', 0, 0/*Capslock*/, CTRL('M'), KEY_INSERT },
	/* 1x */
	{ KEY_F3, '@', 'W', 'S', '+', '2', '3', 'z' },
	{ KEY_F4, '#', 'E', 'D', '-', '5', '6', 'x' },
	/* 2x */
	{ KEY_F5, '$', 'R', 'F', '*', '8', '9', 'c' },
	{ KEY_F6, '%', 'T', 'G', '7', '1', '4', 'v' },
	/* 3x */
	{ KEY_F7, '^', 'Y', 'H', '/', KEY_DOWN, KEY_RIGHT, 'b' },
	{ KEY_F8, '&', 'U', 'J',  0 , KEY_LEFT,  0 , 'n' },
	/* 4x */
	{ KEY_F9, '*', 'I', 'K',  0 ,  0 , KEY_UP, 'm' },
	{ KEY_F10, '(', 'O', 'L', 0, KEY_BS, KEY_ENTER, '<' },
	/* 5x */
	{ KEY_F11, ')', 'P', ':', KEY_DEL, '"', '|', '>' },
	{ KEY_F12, '_', '{', '\'', 0, '+', '}', '?' },
	/* 60 shift 67 ctrl 70 alt */
};


static uint8_t capslock = 0;

static void keydecode_tc(void)
{
	uint8_t c;

	if (keybyte == 1 && keybit == 5) {
		capslock = 1 - capslock;
		return;
	}

	/* TODO: ALT */
	if (keymap[6] & 0x80)	/* shift */
		c = shift_keyboard_tc[keybyte][keybit];
	else
		c = keyboard_tc[keybyte][keybit];

	if (keymap[7] & 0x80) {	/* control */
		if (c > 31 && c < 127)
			c &= 31;
	}
	if (capslock && c >= 'a' && c <= 'z')
		c -= 'a' - 'A';
	tty_inproc(1, c);
}

#if 0
void kbd_interrupt(void)
{
	newkey = 0;
	keyproc();
	if (keysdown < 3 && newkey)
		keydecode();
}

#endif

__sfr __at 0x02	tc256_kstat;
__sfr __at 0x18 tc256_kcode;

/* In order to make most of the code follow the same logic as the scan
   keyboad we basically fake a scan keyboard using the keycodes we get */
static void keymap_down(uint8_t c)
{
	keybyte = c >> 3;
	keybit = c & 7; 
	if (keybyte < 15) {
		keymap[keybyte] |= keybit;
		keysdown++;
		newkey = 1;
	}
}

static void keymap_up(uint8_t c)
{
	if (keybyte < 15) {
		keymap[c >> 3] &= ~(c & 7);
		keysdown--;
	}
}

/* 256TC */
void kbd_interrupt(void)
{
	uint8_t x = tc256_kcode;
	newkey = 0;
	if (x & 0x80)
		keymap_down(x & 0x7F);
	else
		keymap_up(x & 0x7F);
	if (keysdown < 3 && newkey)
		keydecode_tc();
}
