#include <kernel.h>
#include <tty.h>
#include <vt.h>
#include <input.h>
#include <devinput.h>
#include <keycode.h>
#include <zxkey.h>

#define SYMCOL	7
#define SYMROW	3
#define CAPSCOL	5
#define CAPSROW	4

static uint8_t keysdown;
static uint8_t newkey;
static uint8_t ctrl;
uint16_t keybits;
static uint8_t kbd_timer;
static uint8_t keybuf[5] = { 0xff, 0xff, 0xff, 0xff, 0xff };
uint8_t keymap[5] = { 0xff, 0xff, 0xff, 0xff, 0xff };

/*
 *	Portable C equivalent of zxkey.s for bigger machines
 */

uint8_t keyboard[5][8] = {
	/* Decode 0: 0xFEFF */
	{ 'b', 'h', 'v', 'y', 'g', '6', 't', '5' },
	/* Decode 1: 0xFDFF */
	{ 'n', 'j', 'c', 'u', 'f', '7', 'r', '4' },
	/* Decode 2: 0xFBFF */
	{ 'm', 'k', 'x', 'i', 'd', '8', 'e', '3' },
	/* Decode 3: 0xF7FF */
	{ 0 /*SS */ , 'l', 'z', 'o', 's', '9', 'w', '2' },
	/* Decode 4: 0xEFFF */
	{ ' ', 10, 0 /* CS */ , 'p', 'a', '0', 'q', '1' }
};

uint8_t shiftkeyboard[5][8] = {
	/* Decode 0: 0xFEFF */
	{ '*', '^', '/', '[', '}', '&', '>', '%' },
	/* Decode 1: 0xFDFF */
	{ ',', '-', '?', ']', '{', '\'', '<', '$' },
	/* Decode 2: 0xFBFF */
	{ 'm', '+', '$', 'i', '\\', '(', 'e', '#' },	/* FIXME pound not dollar */
	/* Decode 3: 0xF7FF */
	{ 0 /* SS */ , '=', ':', ';', '[', ')', 'w', '@' },
	/* Decode 4: 0xEFF */
	{ ' ', KEY_ENTER, 0 /* CS */ , '"', '~', '_', 'q', '!' }
};


static uint8_t key_is_shift(uint8_t row, uint8_t col)
{
	if (row == SYMROW && col == SYMCOL)
		return 1;
	if (row == CAPSROW && col == CAPSCOL)
		return 1;
	return 0;
}

/* Walk the bitmask and work out what keys went up or down */
static void zxkey_eval(uint8_t delta, uint8_t row, uint8_t kbuf)
{
	uint8_t i;
	for (i = 0; delta; i++, delta <<= 1, kbuf <<= 1) {
		if (key_is_shift(row, i))
			continue;
		if (!(delta & 0x80))
			continue;
		if (kbuf & 0x80) {	/* Up ? */
			if (keyboard_grab == 3) {
				queue_input(KEYPRESS_UP);
				queue_input(keyboard[row][i]);
			}
			keysdown--;
			continue;
		}
		newkey = 1;
		keybits = (row << 8) | i;
		keysdown++;
	}
}

/*
 *	Hand back the key that was pressed along with the modifiers. This
 *	is done in a slightly odd way to match the asm versions
 */
static uint16_t keydecode(void)
{
	uint8_t caps = 0;
	uint8_t sym = 0;
	uint16_t mode = KEYPRESS_DOWN;
	uint8_t ch;

	/* Low is pressed */
	if (!(keybuf[CAPSROW] & CAPSCOL)) {
		mode |= KEYPRESS_SHIFT;
		caps = 0;
	}
	if (!(keybuf[SYMROW] & SYMCOL)) {
		mode |= KEYPRESS_ALT;
		sym = 0;
	}

	/* Symbol + foo is looked up in the shift table */
	if (sym && !caps) {
		ch = shiftkeyboard[keybits >> 8][keybits & 0xFF];
	} else {
		/* Caps|Sym is control and a toggle due to rollover */
		if (sym) {
			ctrl ^= 1;
			return 0;
		}
		/* Normal characters with our without caps are handled in the
		   main table */
		ch = keyboard[keybits >> 8][keybits & 0xFF];
		if (caps) {
			if (ch >= 'a' && ch <= 'z')
				ch -= 32;
			if (ch == '0')
				ch = KEY_BS;
			if (ch == ' ')
				ch = KEY_STOP;
			if (ch >= '1' && ch <= '4')
				return 0x4000 | (ch - '0');	/* Special flag console switch */
			/* TODO: should we map caps 5-8 to cursor ? */
		}
	}
	/* Apply control key */
	if (ctrl) {
		ch &= 0x1F;
		mode |= KEYPRESS_CTRL;
	}
	/* Report the event code bits and the character */
	return (mode << 8) | ch;
}

/*
 *	Read the 5 rows of the keyboard and look for changes. If we find
 *	any then decode the effect.
 */
uint16_t zxkey_scan(void)
{
	/* EFFF F7FF FBFF FDFF FEFF */
	uint16_t port = 0xEFFF;
	uint8_t *keyp = keybuf;
	uint8_t *kmp;
	uint8_t r;
	uint8_t i;
	uint8_t changed = 0;

	newkey = 0;

	/*  Read the ports and update keybuf. If we see a change
	   remember the fact */
	do {
		r = in16(port);
		if (r != *keyp) {
			*keyp = r;
			changed = 1;
		}
		port >>= 1;
		port |= 0x8000;
	} while (port != 0xFFFF);

	/* No change so quick exit */
	if (changed == 0)
		return 0;

	keyp = keybuf;
	kmp = keymap;

	/* Compare the old and new to find out what changed */
	for (i = 0; i < 5; i++) {
		r = *keyp ^ *kmp;
		if (r) {
			/* Evaluate the changes on this line */
			zxkey_eval(r, i, *keyp);
			*kmp++ = *keyp++;
		}
	}
	/* No keys, or too many keys for a rollover means nothing */
	if (keysdown == 0 || keysdown > 2)
		return 0;
	/* A key press happened */
	if (newkey) {
		kbd_timer = keyrepeat.first;
		keydecode();
		return 1;
	}
	/* Keys are still down - do autorepeat */
	if (--kbd_timer)
		return 0;
	kbd_timer = keyrepeat.continual;
	keydecode();
	return 1;
}

void zxkey_init(void)
{
}
