#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <devtty.h>
#include <keycode.h>
#include <vt.h>
#include <tty.h>

static char tbuf1[TTYSIZ];

uint8_t vtattr_cap;
struct vt_repeat keyrepeat;
static uint8_t kbd_timer;

/* buffer for port scan procedure */
uint8_t keybuf[8];
/* Previous state */
uint8_t keymap[8];

static uint8_t keybyte, keybit;
static uint8_t newkey;
static int keysdown = 0;

uint8_t keyboard[8][5] = {
	{' ', 0  , 'm', 'n', 'b'},
	{13 , 'l', 'k', 'j', 'h'},
	{'p', 'o', 'i', 'u', 'y'},
	{'0', '9', '8', '7', '6'},
	{'1', '2', '3', '4', '5'},
	{'q', 'w', 'e', 'r', 't'},
	{'a', 's', 'd', 'f', 'g'},
	{0  , 'z', 'x', 'c', 'v'}
};

/* SYMBOL SHIFT MODE */
uint8_t shiftkeyboard[8][5] = {
	{' ', 0 , '.',  ',', '*'},
	{13 , '=', '+', '-', '^'},
	{'"', ';', '@', ']', '['},
	{'_', ')', '(', '\'','&'},
	{'!', '@', '#', '$', '%'},
	{'`',  0 ,  0 , '<', '>'},
	{'~' ,'|', '\\','{', '}'},
	{0  , ':', KEY_POUND  , '?', '/'}
};


static uint8_t shiftmask[8] = { 0x02, 0, 0, 0, 0, 0, 0, 0x01 };

struct s_queue ttyinq[NUM_DEV_TTY + 1] = {	/* ttyinq[0] is never used */
	{NULL, NULL, NULL, 0, 0, 0},
	{tbuf1, tbuf1, tbuf1, TTYSIZ, 0, TTYSIZ / 2},
};

/* tty1 is the screen */

/* Output for the system console (kprintf etc) */
void kputchar(char c)
{
	if (c == '\n')
		tty_putc(0, '\r');
	tty_putc(0, c);
}

/* Both console and debug port are always ready */
ttyready_t tty_writeready(uint8_t minor)
{
	minor;
	return TTY_READY_NOW;
}

void tty_putc(uint_fast8_t minor, unsigned char c)
{
	minor;
	vtoutput(&c, 1);
}

int tty_carrier(uint_fast8_t minor)
{
	minor;
	return 1;
}

void tty_setup(uint_fast8_t minor, uint_fast8_t wait)
{
	minor;
}

void tty_sleeping(uint_fast8_t minor)
{
	minor;
}

void tty_data_consumed(uint_fast8_t minor)
{
}

void update_keyboard(void)
{
	/* We need this assembler code because SDCC __sfr cannot handle 16-bit addresses. And because it is much faster, of course  */
	/* TODO: make it naked? */
	__asm
		ld hl,#_keybuf
		ld c, #0xFE
		ld b, #0x7f
		ld e, #8        ; 8 keyboard ports, 7FFE, BFFE, DFFE and so on
	read_halfrow:
		in a, (c)
;		and #0
		cpl
		ld(hl), a
		rrc b
		inc hl
		dec e
		jr nz, read_halfrow
	__endasm;
}

void tty_pollirq(void)
{
	int i;

	update_keyboard();

	newkey = 0;

	for (i = 0; i < 8; i++) {
		int n;
		uint8_t key = keybuf[i] ^ keymap[i];
		if (key) {
			uint8_t m = 0x10;
			for (n = 4; n >= 0; n--) {
				if ((key & m) && (keymap[i] & m))
					if (!(shiftmask[i] & m))
						keysdown--;

				if ((key & m) && !(keymap[i] & m)) {
					if (!(shiftmask[i] & m)) {
						keysdown++;
						newkey = 1;
						keybyte = i;
						keybit = n;
					}
				}
				m >>= 1;
			}
		}
		keymap[i] = keybuf[i];
	}
	if (keysdown && keysdown < 3) {
		if (newkey) {
			keydecode();
			kbd_timer = keyrepeat.first;
		} else if (! --kbd_timer) {
			keydecode();
			kbd_timer = keyrepeat.continual;
		}
	}

}

static uint8_t cursor[4] = { KEY_LEFT, KEY_DOWN, KEY_UP, KEY_RIGHT };

static void keydecode(void)
{
	uint8_t c;

	uint8_t ss = keymap[0] & 0x02;	/* SYMBOL SHIFT */
	uint8_t cs = keymap[7] & 0x01;	/* CAPS SHIFT */

	if (ss) {
		c = shiftkeyboard[keybyte][keybit];
	} else {
		c = keyboard[keybyte][keybit];
		if (cs) {
			if (c >= 'a' && c <= 'z')
				c -= 'a' - 'A';
			else if (c == '0')	/* CS + 0 is backspace) */
				c = 0x08;
			else if (c == ' ')
				c = KEY_STOP;	/* ^C map for BREAK */
			else if (c >= '5' && c <= '8')
				c = cursor[c - '5'];
		}
	}
	tty_inproc(1, c);
}


/* This is used by the vt asm code, but needs to live in the kernel */
uint16_t cursorpos;
