#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <keycode.h>
#include <vt.h>
#include <tty.h>
#include <input.h>
#include <devinput.h>

/* buffer for port scan procedure */
uint8_t keybuf[8];
/* Previous state */
uint8_t keymap[8];

struct vt_repeat keyrepeat = { 50, 5 };

static uint8_t kbd_timer;

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

static uint8_t update_keyboard(void) __naked
{
	/*
	 *	This is run 50 time a second so we do it in asm and also return
	 *	0 if nothing changed. That allows us to avoid the main tty
	 *	processing on most interrupt events which saves us a lot of
	 *	clocks.
	 *
	 *	FIXME: optimise out use of e in favour of rrc b c flag clear
	 */
	__asm
		ld hl,#_keybuf
		ld c, #0xFE
		ld b, #0x7f
		ld de, #8        ; 8 keyboard ports, 7FFE, BFFE, DFFE and so on
				 ; D to 0 for no change found
	read_halfrow:
		in a, (c)
		cpl
		cp (hl)
		jr z,nochange
		inc d		; there 8 ports so we cannot overflow
		ld (hl), a
	nochange:
		rrc b
		inc hl
		dec e
		jr nz, read_halfrow
		ld l,d
		ret
	__endasm;
}

static uint8_t cursor[4] = { KEY_LEFT, KEY_DOWN, KEY_UP, KEY_RIGHT };

static void keydecode(void)
{
	uint8_t m = 0;
	uint8_t c;

	uint8_t ss = keymap[0] & 0x02;	/* SYMBOL SHIFT */
	uint8_t cs = keymap[7] & 0x01;	/* CAPS SHIFT */

	if (ss && !cs) {
		m = KEYPRESS_SHIFT;
		c = shiftkeyboard[keybyte][keybit];
	} else {
		c = keyboard[keybyte][keybit];
		if (cs && !ss) {
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
	if (ss && cs) {
		m |= KEYPRESS_CTRL;
		c &= 31;
	}
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


void tty_pollirq(void)
{
	int i;

	newkey = 0;

	/* Nothing changed, and chance of key repeat work - so done */
	if (!update_keyboard() && !keysdown)
		return;

	for (i = 0; i < 8; i++) {
		int n;
		uint8_t key = keybuf[i] ^ keymap[i];
		if (key) {
			uint8_t m = 0x10;
			for (n = 4; n >= 0; n--) {
				if ((key & m) && (keymap[i] & m))
					if (!(shiftmask[i] & m)) {
						if (keyboard_grab == 3) {
							queue_input(KEYPRESS_UP);
							queue_input(keyboard[i][n]);
						}
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
