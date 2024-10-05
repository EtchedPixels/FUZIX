#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <devtty.h>
#include <vt.h>
#include <tty.h>
#include "msx2.h"

#undef  DEBUG			/* UNdefine to delete debug code sequences */

extern void set_active_vt(uint8_t curtty);
extern void set_visible_vt(uint8_t curtty);

__sfr __at 0x2F tty_debug;
__sfr __at 0xAA kbd_row_set;
__sfr __at 0xA9 kbd_row_read;

static uint8_t tbuf1[TTYSIZ];
static uint8_t tbuf2[TTYSIZ];
static uint8_t tbuf3[TTYSIZ];
static uint8_t tbuf4[TTYSIZ];
static uint8_t tbuf5[TTYSIZ];

uint8_t curtty;
uint8_t inputtty;
static struct vt_switch ttysave[NUM_DEV_TTY];
struct vt_repeat keyrepeat;
uint8_t vtattr_cap;
static uint8_t kbd_timer;

struct s_queue ttyinq[NUM_DEV_TTY + 1] = {	/* ttyinq[0] is never used */
	{NULL, NULL, NULL, 0, 0, 0},
	{tbuf1, tbuf1, tbuf1, TTYSIZ, 0, TTYSIZ / 2},
	{tbuf2, tbuf2, tbuf2, TTYSIZ, 0, TTYSIZ / 2},
	{tbuf3, tbuf3, tbuf3, TTYSIZ, 0, TTYSIZ / 2},
	{tbuf4, tbuf4, tbuf4, TTYSIZ, 0, TTYSIZ / 2},
};

static tcflag_t console_mask[4] = {
	_ISYS,
	_OSYS,
	_CSYS,
	_LSYS
};

tcflag_t termios_mask[NUM_DEV_TTY + 1] = {
	0,
	_CSYS,
	_CSYS,
	_CSYS,
	_CSYS
};

uint8_t keyboard[11][8];
uint8_t shiftkeyboard[11][8];

/* tty1 to tty4 is the screen */

/* Output for the system console (kprintf etc) */
static void
kputc(uint_fast8_t minor, uint_fast8_t c)
{
	/* Debug port for bringup */
	tty_debug = c;
	tty_putc(minor, c);
}

void kputchar(uint_fast8_t c)
{
	if (c == '\n')
		kputc(minor(TTYDEV), '\r');
	kputc(minor(TTYDEV), c);
}

/* All tty are always ready */
ttyready_t tty_writeready(uint_fast8_t minor)
{
	minor;
	// tty are ready to write
	return TTY_READY_NOW;
}

void vtexchange()
{
	vt_save(&ttysave[curtty]);
	cursor_off();
	set_visible_vt(inputtty);
	cursor_on(ttysave[inputtty].cursory, ttysave[inputtty].cursorx);
}


void tty_putc(uint_fast8_t minor, uint_fast8_t c)
{
	irqflags_t irq;

	irq = di();
	if (curtty != minor -1) {
		vt_save(&ttysave[curtty]);
		curtty = minor - 1;
		vt_load(&ttysave[curtty]);
		set_active_vt(curtty);
	}
	vtoutput(&c, 1);
	irqrestore(irq);
}

int tty_carrier(uint_fast8_t minor)
{
	minor;
	return 1;
}

void tty_data_consumed(uint_fast8_t minor)
{
}

void tty_setup(uint_fast8_t minor, uint_fast8_t flags)
{
	minor;

	/* setup termios to use msx keys: FIXME */
	ttydata[minor].termios.c_cc[VERASE] = KEY_BS;
	ttydata[minor].termios.c_cc[VSTOP] = KEY_STOP;
	ttydata[minor].termios.c_cc[VSTART] = KEY_STOP;
}

uint8_t keymap[11];
static uint8_t keyin[11];
static uint8_t keybyte, keybit;
static uint8_t newkey;
static int keysdown = 0;
static uint8_t modmask[11] = { /* check for SHIFT, CTRL, GRPH and CODE */
	0, 0, 0, 0, 0, 0, 0x17, 0, 0, 0, 0
};

static void keyproc(void)
{
	int i;
	uint8_t key;

	if ((keyin[6] ^ keymap[6]) & modmask[6])
		kbd_timer = keyrepeat.first; /* modifier state change invalidates repeat timer */
	for (i = 0; i < 11; i++) {
		key = keyin[i] ^ keymap[i];
		if (key) {
			int n;
			int m = 1;
			for (n = 0; n < 8; n++) {
				if ((key & m) && (keymap[i] & m)) {
					if (!(modmask[i] & m))
						keysdown--;
				}
				if ((key & m) && !(keymap[i] & m)) {
					if (!(modmask[i] & m)) {
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

static uint8_t capslock = 0;

static void keydecode(void)
{
	uint8_t c;

	if (keybyte == 6 && keybit == 3) {
		capslock = 1 - capslock;
		return;
	}

	if (keymap[6] & 3 ) {	/* shift or control */
		c = shiftkeyboard[keybyte][keybit];
		/* VT switcher */
		if (c == KEY_F1 || c == KEY_F2 || c == KEY_F3 || c == KEY_F4) {
			if (inputtty != c - KEY_F1) {
				inputtty = c - KEY_F1;
				vtexchange();	/* Exchange the video and backing buffer */
			}
			return;
		}
	} else
		c = keyboard[keybyte][keybit];

	/* Until we have true i8n we should make sure the Yen sign is interpreted as backslash */
	if (c == KEY_YEN)
		c = '\\';

	if (keymap[6] & 2) {	/* control */
		if (c > 31 && c < 127)
			c &= 31;
	}

	if (capslock && c >= 'a' && c <= 'z')
		c -= 'a' - 'A';

	/* TODO: function keys (F1-F10), graph, code */

	vt_inproc(inputtty +1, c);
}

void update_keyboard(void)
{
	int n;
	uint8_t r;

	/* encode keyboard row in bits 0-3 0xAA, then read status from 0xA9 */
	for (n =0; n < 11; n++) {
		r = kbd_row_set & 0xf0 | n;
		kbd_row_set = r;
		keyin[n] = ~kbd_row_read;
	}
}


void kbd_interrupt(void)
{
	newkey = 0;
	update_keyboard();
	keyproc();

	/* accept any number of keys down, only retain last one pressed */
	if (keysdown > 0) {
		if (newkey) {
			keydecode();
			kbd_timer = keyrepeat.first;
		} else if (! --kbd_timer) {
			keydecode();
			kbd_timer = keyrepeat.continual;
		}
	}
}

void tty_sleeping(uint_fast8_t minor)
{
    minor;
}

/* This is used by the vt asm code, but needs to live in the kernel */
uint16_t cursorpos;
