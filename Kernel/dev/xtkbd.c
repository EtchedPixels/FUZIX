/*
 *	PC/XT keyboard interface for raw scancodes
 *
 *	EO is a prefix for certain keys on extended keyboards
 *	E0 1C is keypad enter
 *	E0 1D is right control
 *	E0_37 is ctrl-printscr
 *	EO 38 is right alt
 *	E0 46 is ctrl-break
 *	E0 47 is grey-home
 *	E0 48 is grey up
 *	E0 4B is grey left
 *	E0 4D is grey right
 *	E0 4F is grey end
 *	E0 50 is grey down
 *	E0 51 is grey pagedown
 *	E0 52 is grey insert
 *	E0 53 is grey delete
 *
 *	Most of these we can ignore the E0
 *
 *
 */

#include <kernel.h>
#include <timer.h>
#include <tty.h>
#include <vt.h>
#include <input.h>
#include <keycode.h>
#include <devinput.h>
#include <xtkbd.h>
#include <printf.h>

static int present;

#ifdef CONFIG_VT_MULTI
extern uint8_t inputtty;	/* FIXME */
#else
#define inputtty 1
#endif

#define CONTROL		29
#define LSHIFT		42
#define RSHIFT		54
#define ALT		56	/* E056 is ALTGR */
#define CAPSLOCK	58
#define NUMLOCK		69

static uint8_t keymap[128] = {
	0, KEY_ESC, '1', '2', '3', '4', '5', '6',
	'7', '8', '9', '0', '-', '=', KEY_BS, KEY_TAB,
	/* 0x10 */
	'q', 'w', 'e', 'r', 't', 'y', 'u', 'i',
	'o', 'p', '[', ']', KEY_ENTER, 0 /* LCTRL */ , 'a', 's',
	/* 0x20 */
	'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',
	'"', '~', 0 /* LSHIFT */ , '\\' /* 102 key only */ , 'z', 'x', 'c',
	'v',
	/* 0x30 */
	'b', 'n', 'm', ',', '.', '/', 0 /* RSHIFT */ ,
	'*' /* PRTSCR 83-84 key */ ,
	0 /* LALT */ , ' ', 0 /* CAPSLOCK */ , KEY_F1, KEY_F2, KEY_F3,
	KEY_F4, KEY_F5,
	/* 0x40 */
	KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10, 0 /* NUMOCK */ ,
	0 /* SCROLL LOCK */ , KEY_HOME,
	KEY_UP, KEY_PGUP, '-' /* Keypad */ , KEY_LEFT, 0 /* KP5 */ ,
	KEY_RIGHT, '+' /* Keypad */ , KEY_END,
	/* 0x50 */
	KEY_DOWN, KEY_PGDOWN, KEY_INSERT, KEY_DEL,
	    /* The following are only found on some extended keyboards */
	    0 /* ALT_SYSRQ */ , 0, 0 /* non US dead */ , KEY_F11, KEY_F12	/* 101+ key */
	    /* Beyond this it gets a bit random */
};

static uint8_t keymap_shift[128] = {
	0, KEY_ESC, '!', '@', '#', '$', '%', '^',
	'&', '*', '(', ')', '_', '+', KEY_BS, KEY_TAB,
	/* 0x10 */
	'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I',
	'O', 'P', '{', '}', KEY_ENTER, 0 /* LCTRL */ , 'A', 'S',
	/* 0x20 */
	'D', 'F', 'G', 'H', 'J', 'K', 'L', ':',
	'`', '~', 0 /* LSHIFT */ , '|' /* 102 key only */ , 'Z', 'X', 'C',
	'V',
	/* 0x30 */
	'B', 'N', 'M', '<', '>', '?', 0 /* RSHIFT */ ,
	'*' /* PRTSCR 83-84 key */ ,
	0 /* LALT */ , ' ', 0 /* CAPSLOCK */ , KEY_F1, KEY_F2, KEY_F3,
	KEY_F4, KEY_F5,
	/* 0x40 */
	KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10, 0 /* NUMOCK */ ,
	0 /* SCROLL LOCK */ , '7',
	'8', '9', '-' /* Keypad */ , '4', '5' /* KP5 */ , '6',
	'+' /* Keypad */ , '1',
	/* 0x50 */
	'2', '3', '4', '.',
	/* The following are only found on some extended keyboards */
	0 /* ALT_SYSRQ */ ,0 ,0 /* non US dead */ , KEY_F11, KEY_F12	/* 101+ key */
};

static uint8_t shift_down;
static uint8_t ctrl_down;
static uint8_t alt_down;
static uint8_t capslock;
static uint8_t numlock;

static uint8_t alpha(uint8_t c)
{
	if (c >= 'A' && c <= 'Z')
		return 1;
	if (c >= 'a' && c <= 'z')
		return 1;
	return 0;
}

void xtkbd_keycode(uint_fast8_t code)
{
	static uint_fast8_t brk;
	uint_fast8_t key;
	uint_fast8_t m = 0;
	uint8_t *table = keymap;
	static uint_fast8_t was_e0;
	unsigned int shifted = 0;

	if (brk) {
		--brk;
		/* TODO when brk hits 0 report it properly */
		return;
	}

	/* The pause/brk key and the weird add on multimedia keys */
	if (code == 0xE1) {
		brk = 6;
		return;
	}
	/* A stray init */
	if (code == 0xAA)
		return;

	/* Lost char/error */
	if (code == 0xFF)
		return;

	if (code == 0xE0) {
		was_e0 = 1;
		return;
	}
	shifted = shift_down;

	/* Number lock */
	if (numlock && code >= 0x46 && code < 0x54)
		shifted = !shifted;

	/* Shift */
	if (shifted)
		table = keymap_shift;

	/* Key up event - track shifts */
	if (code & 0x80) {
		if (keyboard_grab == 3) {
			queue_input(KEYPRESS_UP);
			queue_input(code & 0x7F);
		}
		if (code == LSHIFT)
			shift_down &= ~1;
		else if (code == RSHIFT)
			shift_down &= ~2;
		/* The right forms of these are E0 prefixed */
		else if (code == CONTROL)
			ctrl_down &= ~(1 << was_e0);
		else if (code == ALT)
			alt_down &= ~(1 << was_e0);
		/* caps lock, shift and friends all send autorepeat so care needed */
		if (code == CAPSLOCK)
			capslock ^= 1;	/* On the up toggle capslock */
		was_e0 = 0;
		return;
	}
	/* Track shift key downs */
	if (code == LSHIFT)
		shift_down |= 1;
	else if (code == RSHIFT)
		shift_down |= 2;
	else if (code == CONTROL)
		ctrl_down |= (1 << was_e0);
	else if (code == ALT)
		alt_down |= (1 << was_e0);
	key = table[code];
#ifdef CONFIG_VT_MULTI
	if (alt_down && key >= KEY_F1 && key <= KEY_F12) {
		xtkbd_conswitch(key - KEY_F1 + 1);
		return;
	}
#endif


/*    kprintf("Code %d Key %d KG %d IT %d\n",  *
 *        code, key, keyboard_grab, inputtty); */

	if (shift_down)
		m = KEYPRESS_SHIFT;
	if (ctrl_down)
		m |= KEYPRESS_CTRL;
	key &= 31;
	if (alt_down)
		m |= KEYPRESS_ALT;
	if (capslock && alpha(key))
		key ^= 32;
	if (key) {
		switch (keyboard_grab) {
		case 0:
			vt_inproc(inputtty, key);
			break;
		case 1:
			if (!input_match_meta(key)) {
				vt_inproc(inputtty, key);
				break;
			}
			/* Fall through */
		case 2:
			queue_input(KEYPRESS_DOWN);
			queue_input(key);
			break;
		case 3:
			queue_input(KEYPRESS_DOWN | m);
			queue_input(code);
			break;
		}
	}
	was_e0 = 0;
}

int xtkbd_init(void)
{
	timer_t timeout;
	irqflags_t irq;
	unsigned int code;
	
	xtkbd_reset();
	timeout = set_timer_sec(3);
	while (timer_expired(timeout)) {
		irq = di();
		code = xtkbd_read();
		irqrestore(irq);
		if (code == 0xAA) {
			present = 1;
			return present;
		}
	}
	return 0;
}
