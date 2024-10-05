#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <tty.h>
#include <vt.h>
#include <graphics.h>
#include <input.h>
#include <devtty.h>
#include <stdarg.h>

static uint8_t vmode = 2;

__sfr __at 252 vmpr;

static uint8_t tbuf1[TTYSIZ];
static uint8_t tbuf2[TTYSIZ];

struct vt_repeat keyrepeat = { 50, 5 };

struct  s_queue  ttyinq[NUM_DEV_TTY+1] = {       /* ttyinq[0] is never used */
    {   NULL,    NULL,    NULL,    0,        0,       0    },
    {   tbuf1,   tbuf1,   tbuf1,   TTYSIZ,   0,   TTYSIZ/2 },
    {   tbuf2,   tbuf2,   tbuf2,   TTYSIZ,   0,   TTYSIZ/2 }
};

tcflag_t termios_mask[NUM_DEV_TTY + 1] = {
	0,
	_CSYS,
	_CSYS		/* For now */
};

/* Write to system console */
void kputchar(uint_fast8_t c)
{
    if(c=='\n')
        tty_putc(1, '\r');
    tty_putc(1, c);
}

uint_fast8_t tty_writeready(uint_fast8_t minor)
{
    return TTY_READY_NOW;
}

void tty_putc(uint_fast8_t minor, uint_fast8_t c)
{
    if (minor == 1) {
        if (vmode == 2)
	        vtoutput(&c, 1);
    }
}

void tty_interrupt(void)
{
}

void tty_setup(uint_fast8_t minor, uint_fast8_t flags)
{
}

int tty_carrier(uint_fast8_t minor)
{
    return 1;
}

void tty_sleeping(uint_fast8_t minor)
{
}

void tty_data_consumed(uint_fast8_t minor)
{
}

uint8_t keymap[9];
uint8_t keyin[9];
static uint8_t keybyte, keybit;
static uint8_t newkey;
static int keysdown = 0;
static uint8_t shiftmask[9] = {
    0x01, 0, 0, 0, 0, 0, 0, 0x02, 0x01
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
					if (!(shiftmask[i] & m)) {
						if (keyboard_grab == 3) {
							queue_input(KEYPRESS_UP);
							queue_input(keyboard[i][n]);
						}
						keysdown--;
					}
				}
				if ((key & m) && !(keymap[i] & m)) {
					if (!(shiftmask[i] & m)) {
						keysdown++;
						keybyte = i;
						keybit = n;
						newkey = 1;
					}
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
uint8_t keyboard[9][8] = {
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

uint8_t shiftkeyboard[9][8] = {
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
	uint8_t m = 0;
	
	/* We don't do anything clever for both shifts or other weird combos
	   This computer isn't going to run emacs after all */

	 
	if (keymap[7] & 0x02) {	/* Symbol Shift */
		c = altkeyboard[keybyte][keybit];
		m = KEYPRESS_ALT;
	} else if (keymap[0] & 0x01) {
		c = shiftkeyboard[keybyte][keybit];
		m = KEYPRESS_SHIFT;
	} else
		c = keyboard[keybyte][keybit];

	if (c == KEY_CAPSLOCK) {
		capslock = 1 - capslock;
		return;
	}
	if (keymap[8] & 0x01) {	/* control */
		m |= KEYPRESS_CTRL;
		/* These map the SAM specific behaviours */
		if (c == KEY_LEFT)
			c = KEY_HOME;
		else if (c == KEY_RIGHT)
			c = KEY_END;
		else
			c &= 31;
	}
	if (capslock && c >= 'a' && c <= 'z')
		c -= 'a' - 'A';
	if (c) {
		switch (keyboard_grab) {
		case 0:
			vt_inproc(1, c);
			break;
		case 1:
			if (!input_match_meta(c)) {
				vt_inproc(1, c);
				break;
			}
			/* Fall through */
		case 2:
			queue_input(KEYPRESS_DOWN);
			queue_input(c);
			break;
		case 3:
			/* Queue an event giving the base key (unshifted)
			   and the state of shift/ctrl/alt */
			queue_input(KEYPRESS_DOWN | m);
			queue_input(keyboard[keybyte][keybit]);
			break;
		}
	}
}

static uint8_t kbd_timer;

void kbd_interrupt(void)
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

/*
 *	Graphics routines. Not yet very useful.
 */

/* Should be const but then some sdcc's put it in the wrong segment */
static struct display samdisplay[4] = {
	/* Mode 1 (0 to us): Spectrum mode */
	{
		0,
		256, 192,
		256, 192,
		0xFF, 0xFF,
		FMT_SPECTRUM,
		HW_UNACCEL,
		GFX_VBLANK,	/* Deal with palette later */
		0,		/* To be added */
	},
	/* Mode 2 (attribute per character row mode) */
	{
		1,
		256, 192,
		256, 192,
		0xFF, 0xFF,
		FMT_SAM2,
		HW_UNACCEL,
		GFX_VBLANK,
		0,
	},
	/* Mode 3 (512 x 192 4 2bp) */
	{
		2,
		512, 192,
		512, 192,
		0xFF, 0xFF,
		FMT_COLOUR4,
		HW_UNACCEL,
		GFX_VBLANK|GFX_TEXT,
		0,
	},
	/* Mode 4 (256 x 192 16 colour) */
	{
		3,
		256, 192,
		256, 192,
		0xFF, 0xFF,
		FMT_COLOUR16,
		HW_UNACCEL,
		GFX_VBLANK,
		0,
	}
};

/*
 *	Graphics ioctls. At minimum we need to extend this to support
 *	GETPALETTE/SETPALETTE and some read/write/copy ops as we can't
 *	sanely map the video into a user process.
 *
 *	We keep the full 24K allocated in a fixed place. When we switch modes
 *	we don't try and do any clever reclaiming.
 */
int gfx_ioctl(uint_fast8_t minor, uarg_t arg, char *ptr)
{
	uint8_t m;
	if (minor != 1 || arg >> 8 != 0x03)
		return vt_ioctl(minor, arg, ptr);
	switch(arg) {
	case GFXIOC_GETINFO:
		return uput(&samdisplay[vmode], ptr, sizeof(struct display));
	case GFXIOC_GETMODE:
	case GFXIOC_SETMODE:
		m = ugetc(ptr);
		if (m > 3) {
			udata.u_error = EINVAL;
			return -1;
		}
		if (arg == GFXIOC_GETMODE)
			return uput(&samdisplay[m], ptr, sizeof(struct display));
		vmode = m;
		vmpr = (vmpr & 0x9F) | (m << 5);
		return 0;
	}
	return -1;
}
