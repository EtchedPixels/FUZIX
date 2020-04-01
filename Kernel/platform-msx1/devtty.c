#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <devtty.h>
#include <vt.h>
#include <tty.h>
#include <devtty.h>

#undef  DEBUG			/* UNdefine to delete debug code sequences */

__sfr __at 0x2F tty_debug2;
__sfr __at 0xAA kbd_row_set;
__sfr __at 0xA9 kbd_row_read;

uint8_t keyboard[11][8];
uint8_t shiftkeyboard[11][8];
uint8_t keymap[11];

struct vt_repeat keyrepeat;
uint8_t vtattr_cap;

static char tbuf1[TTYSIZ];
static char tbuf2[TTYSIZ];

struct s_queue ttyinq[NUM_DEV_TTY + 1] = {	/* ttyinq[0] is never used */
	{NULL, NULL, NULL, 0, 0, 0},
	{tbuf1, tbuf1, tbuf1, TTYSIZ, 0, TTYSIZ / 2},
	{tbuf2, tbuf2, tbuf2, TTYSIZ, 0, TTYSIZ / 2}
};

tcflag_t termios_mask[NUM_DEV_TTY + 1] = {
	0,
	_CSYS,
	_CSYS
};


uint8_t vtattr_cap = 0;		/* For now */

/* tty1 is the screen tty2 is the debug port */

/* Output for the system console (kprintf etc) */
void kputchar(char c)
{
	/* Debug port for bringup */
	if (c == '\n')
		tty_putc(2, '\r');
	tty_putc(2, c);
}

/* Both console and debug port are always ready */
ttyready_t tty_writeready(uint8_t minor)
{
	minor;
	return TTY_READY_NOW;
}

void tty_putc(uint8_t minor, unsigned char c)
{
	minor;
	tty_debug2 = c;	
//	if (minor == 1) {
		vtoutput(&c, 1);
//		return;
//	}
}

int tty_carrier(uint8_t minor)
{
	minor;
	return 1;
}

void tty_setup(uint8_t minor, uint8_t flags)
{
	minor;
}

void tty_sleeping(uint8_t minor)
{
	minor;
}

void tty_data_consumed(uint8_t minor)
{
}

static uint8_t kbd_timer;
static uint8_t keyin[11];
static uint8_t keybyte, keybit;
static uint8_t newkey;
static int keysdown = 0;
static uint8_t shiftmask[11] = {
	0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0
};

static void keyproc(void)
{
	int i;
	uint8_t key;

	for (i = 0; i < 11; i++) {
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
#if 0		
		if (c == KEY_F1 || c == KEY_F2 || c == KEY_F3 || c == KEY_F4) {
			if (inputtty != c - KEY_F1) {
				inputtty = c - KEY_F1;
				vtexchange();	/* Exchange the video and backing buffer */
			}
			return;
		}
#endif			
	} else
		c = keyboard[keybyte][keybit];

	if (keymap[6] & 2) {	/* control */
		if (c > 31 && c < 127)
			c &= 31;
	}

	if (capslock && c >= 'a' && c <= 'z')
		c -= 'a' - 'A';

	/* TODO: function keys (F1-F10), graph, code */

	vt_inproc(/*inputtty +*/1, c);
}

void update_keyboard()
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

	if (keysdown && keysdown < 3) {
		if (newkey) {
			keydecode();
			kbd_timer = keyrepeat.first * ticks_per_dsecond;
		} else if (! --kbd_timer) {
			keydecode();
			kbd_timer = keyrepeat.continual * ticks_per_dsecond;
		}
	}
}

/* This is used by the vt asm code, but needs to live in the kernel */
uint16_t cursorpos;
