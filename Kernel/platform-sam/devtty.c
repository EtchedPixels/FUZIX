#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <tty.h>
#include <vt.h>
#include <devtty.h>
#include <stdarg.h>

char tbuf1[TTYSIZ];
char tbuf2[TTYSIZ];

struct  s_queue  ttyinq[NUM_DEV_TTY+1] = {       /* ttyinq[0] is never used */
    {   NULL,    NULL,    NULL,    0,        0,       0    },
    {   tbuf1,   tbuf1,   tbuf1,   TTYSIZ,   0,   TTYSIZ/2 },
    {   tbuf2,   tbuf2,   tbuf2,   TTYSIZ,   0,   TTYSIZ/2 }
};

static tcflag_t console_mask[4] = {
	_ISYS,
	_OSYS,
	_CSYS,
	_LSYS
};

tcflag_t *termios_mask[NUM_DEV_TTY + 1] = {
	NULL,
	console_mask,
	console_mask		/* For now */
};

/* Write to system console */
void kputchar(char c)
{
    if(c=='\n')
        tty_putc(1, '\r');
    tty_putc(1, c);
}

uint8_t tty_writeready(uint8_t minor)
{
    return TTY_READY_NOW;
}

void tty_putc(uint8_t minor, unsigned char c)
{
    if (minor == 1)
        vtoutput(&c, 1);
}

void tty_interrupt(void)
{
}

void tty_setup(uint8_t minor, uint8_t flags)
{
}

int tty_carrier(uint8_t minor)
{
    return 1;
}

void tty_sleeping(uint8_t minor)
{
}

void tty_data_consumed(uint8_t minor)
{
}

static uint8_t keymap[9];
uint8_t keyin[9];
static uint8_t keybyte, keybit;
static uint8_t newkey;
static int keysdown = 0;
static uint8_t shiftmask[9] = {
    0x80, 0, 0, 0, 0, 0, 0, 0x40, 0x80,
};

static void keyproc(void)
{
	int i;
	uint8_t key;

	keyscan();

	for (i = 0; i < 9; i++) {
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
					if (!(shiftmask[i] & m))
						keysdown++;
					keybyte = i;
					keybit = n;
					newkey = 1;
				}
				m += m;

			}
		}
		keymap[i] = keyin[i];
	}
}

/*
 *	The SAM keyboard is a bit ... strange. We mitigate it a little by
 *	adding some other mappings in more normal places
 *
 *	Keys with no existing shift mapping
 *	Shifted comma and dot produce <> just like on a real keyboard
 *	Shifted ;: provide [] and symshifted {} 
 *
 *	Extra mappings
 *	sym-shift 1 produces | rather than ! to fix the missing | symbol
 *	sym-shift 7 produces the missing back-quote
 *
 *	Modified mappings
 *	As INV has no meaning and the slash is just wrong we generate / for
 *	INV and \ for shift-INV.
 *
 *	Of course if you don't like it you can load a keymap.
 *
 *	FIXME: you can't currently load the symshift keymap. The core code
 *	doesn't understand having a third translation table.
 */
static uint8_t keyboard[9][8] = {
	{ 0 , 'z', 'x', 'c', 'v', KEY_F1, KEY_F2, KEY_F3 },
	{'a', 's', 'd', 'f', 'g', KEY_F4, KEY_F5, KEY_F6 },
	{'q', 'w', 'e', 'r', 't', KEY_F7, KEY_F8, KEY_F9 },
	{'1', '2', '3', '4', '5', KEY_ESC, KEY_TAB, KEY_CAPSLOCK },
	{'0', '9', '8', '7', '6', '-', '+', KEY_BS },
	{'p', 'o', 'i', 'u', 'y', '=', '"', KEY_F10 },
	{KEY_ENTER, 'l', 'k', 'j', 'h', ';', ':', KEY_EDIT },
	{' ', 0 /* SymShift */, 'm', 'n', 'b', ',', '.', '/' },
	{0/* CTRL*/, KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT, 0, 0, 0 }
};

static uint8_t shiftkeyboard[9][8] = {
	{ 0 , 'Z', 'X', 'C', 'V', KEY_PGUP, KEY_F2, KEY_F3 },
	{'A', 'S', 'D', 'F', 'G', KEY_PGDOWN, KEY_F5, KEY_F6 },
	{'Q', 'W', 'E', 'R', 'T', KEY_F7, KEY_F8, KEY_F9 },
	{'!', '@', '"', '$', '%', KEY_ESC, KEY_TAB, KEY_CAPSLOCK },
	{'~', ')', '(', '\'', '&', '/', '*', KEY_DEL },
	{'P', 'O', 'I', 'U', 'Y', '_', KEY_COPYRIGHT, KEY_F10 },
	{KEY_ENTER, 'L', 'K', 'J', 'H', '[', ']', KEY_EDIT },
	{' ', 0 /* SymShift */, 'M', 'N', 'B', '<', '>', '\\' },
	{0/* CTRL*/, KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT, 0, 0, 0 }
};

static uint8_t altkeyboard[9][8] = {
	{ 0, 0, '?', 0, 0, KEY_F1, KEY_F2, KEY_F3 },
	{ 0, 0, 0, '{', '}', KEY_F4, KEY_F5, KEY_F6 },
	{'<', '>', 0, '[', ']', KEY_F7, KEY_F8, KEY_F9 },
	{'|', '@', '"', '$', '%', KEY_ESC, KEY_TAB, KEY_CAPSLOCK },
	{'~', ')', '(', '`', '&', '/', '*', KEY_DEL },
	{0, 0, 0, 0, 0, '_', KEY_COPY, KEY_F10 },
	{KEY_ENTER, KEY_POUND, 0, 0, '^', '{', '}', KEY_EDIT },
	{' ', 0 /* SymShift */, 'M', 'N', 'B', '<', '>', '\\' },
	{0/* CTRL*/, KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT, 0, 0, 0 }
};


static uint8_t capslock = 0;

static void keydecode(void)
{
	uint8_t c;

	/* We don't do anything clever for both shifts or other weird combos
	   This computer isn't going to run emacs after all */
	if (keymap[7] & 0x02)	/* Symbol Shift */
		c = altkeyboard[keybyte][keybit];
	else if (keymap[0] & 0x01)
		c = shiftkeyboard[keybyte][keybit];
	else
		c = keyboard[keybyte][keybit];

	if (c == KEY_CAPSLOCK) {
		capslock = 1 - capslock;
		return;
	}
        /* The keyboard lacks some rather important symbols so remap them
           with control */
	if (keymap[8] & 0x01) {	/* control */
		/* These map the SAM specific behaviours */
		if (c == KEY_LEFT)
			c = KEY_HOME;
		else if (c == KEY_RIGHT)
			c = KEY_END;
		else if (c > 31 && c < 96)
			c &= 31;
	}
	if (capslock && c >= 'a' && c <= 'z')
		c -= 'a' - 'A';
	if (c)
		vt_inproc(1, c);
}

void kbd_interrupt(void)
{
	newkey = 0;
	keyproc();
	if (keysdown < 3 && newkey)
		keydecode();
}

