/*
 *	The BIOS keyboard interface is pretty much unusable for a Unixlike
 *	OS, so sadly we have to take over the keyboard ourselves. Hopefully
 *	the early non-clones didn't do anything too weird in this space.
 *
 *	Based on the ELKS driver by Saku Airila and Sefaan Van Dooren
 *
 *	We need to look at the following
 *	- Some basic AT support
 *	- Nicer mapping of key codes onto ELKS standard FN etc codes
 *	- Loadable keymaps
 *
 *	The console layer expects us to call console_queue(uint8_t x)
 *	with FUZIX codes for each key. If we see Alt-Fn we call
 *	console_switch(n) to indicate a possible console switch. Other
 *	than that we try to be blissfully ignorant of the tty layer as it's
 *	simply not our problem and we are not the only console driver.
 */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <tty.h>
#include <vt.h>
#include <devtty.h>

#ifdef CONFIG_KBD_XT

#define KBD_IO	0x60
#define KBD_CTL	0x61

#define ESC 27
#define KB_SIZE 64

static uint8_t xtkb_scan[] = {
	0, 033, '1', '2', '3', '4', '5', '6',
	'7', '8', '9', '0', '-', '=', '\b', '\t',
/*10*/	'q', 'w', 'e', 'r', 't', 'y', 'u', 'i',
	'o', 'p', '[', ']', KEY_ENTER, /* lctrl */0x82, 'a', 's',
/*20*/	'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',
	'\'', '#', /* lshift */0x80, '\\', 'z', 'x', 'c', 'v',
	'b', 'n', 'm', ',', '.', '/', /* rshift */0x81, '*',
	/* lalt */0x83, ' ', /* capslock */0x84, KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5,
	KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10, /* numlock */0x85, /* scrollock */0x90, KEY_HOME,
	KEY_UP, KEY_PGUP, KEY_MINUS, KEY_LEFT, /* kb-5 ?? */0265, KEY_RIGHT, KEY_PLUS, KEY_END,
	KEY_DOWN, KEY_PGDOWN, KEY_INSERT, KEY_DEL
};

static uint8_t xtkb_scan_shifted[] = {
	0, 033, '!', '"', '£', '$', '%', '^',
	'&', '*', '(', ')', '_', '+', '\b', '\t',
	'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I',
	'O', 'P', '{', '}', KEY_ENTER, /* lctrl */0x82, 'A', 'S',
	'D', 'F', 'G', 'H', 'J', 'K', 'L', ':',
	'"', '~', /* lshift */0x80, '|', 'Z', 'X', 'C', 'V',
	'B', 'N', 'M', '<', '>', '?', /* rshift */0x81, '*',
	/* FIXME: we don't have shift- and ctrl Fx maps but need them */
	/* lalt*/0x83, ' ', /* capslock */0x84, KEY_F1, KEY_F1, KEY_F3, KEY_F4, KEY_F5,
	KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10, /* numlock */0x84, /* scrollock */0x93, '7',
	'8', '9', /* keypad - */'-', '4', '5', '6', '+', '1',
	'2', '3', '0', KEY_DEL
};

static uint8_t xtkb_scan_ctrl_alt[] = {
	0, 033, '1', '2', '3', '4', '5', '6',
	'7', '8', '9', '0', '-', '=', KEY_BS, KEY_TAB,
	'q', 'w', 'e', 'r', 't', 'y', 'u', 'i',
	'o', 'p', '[', ']', KEY_ENTER, /* lctrl */0x82, 'a', 's',
	'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',
	'\'', '`', /* lshift */0x80, '`', 'z', 'x', 'c', 'v',
	'b', 'n', 'm', ',', '.', '/', /* rshift */0x81, '*',
	/* lalt */0x83, ' ', /* capslock */0x84, KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5,
	KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10, /* numlock */0x85, /* scrollock */0x90, KEY_HOME,
	KEY_UP, KEY_PGUP, KEY_MINUS, KEY_LEFT, /* kb-5 ?? */0265, KEY_RIGHT, KEY_PLUS, KEY_END,
	KEY_DOWN, KEY_PGDOWN, KEY_INSERT, KEY_DEL
};

static uint8_t xtkb_scan_caps[] = {
	0, 033, '1', '2', '3', '4', '5', '6',
	'7', '8', '9', '0', '-', '=', KEY_BS, KEY_TAB,
	'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I',
	'O', 'P', '[', ']', KEY_ENTER, 0x82, 'A', 'S',
	'D', 'F', 'G', 'H', 'J', 'K', 'L', ':',
	'\'', '~', 0x80, '|', 'Z', 'X', 'C', 'V',
	'B', 'N', 'M', ',', '.', '/', 0x81, '*',
	/* lalt*/0x83, ' ', /* capslock */0x84, KEY_F1, KEY_F1, KEY_F3, KEY_F4, KEY_F5,
	KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10, /* numlock */0x84, /* scrollock */0x93, '7',
	'8', '9', /* keypad - */'-', '4', '5', '6', '+', '1',
	'2', '3', '0', KEY_DEL
};

#define LSHIFT 1
#define RSHIFT 2
#define CTRL 4
#define ALT 8
#define CAPS 16
#define NUM 32
#define ALT_GR 64

#define ANYSHIFT (LSHIFT | RSHIFT)

#define SSC 0xC0

static uint8_t tb_state[] = {
	0x80, CTRL, SSC, SSC,	/*1C->1F */
	SSC, SSC, SSC, SSC, SSC, SSC, SSC, SSC,	/*20->27 */
	SSC, SSC, LSHIFT, SSC, SSC, SSC, SSC, SSC,	/*28->2F */
	SSC, SSC, SSC, SSC, SSC, SSC, RSHIFT, SSC,	/*30->37 */
	SSC, SSC, CAPS,		/*38->3A */
	'a', 'b', 'c', 'd', 'e',	/*3B->3F, Function Keys */
	'f', 'g', 'h', 'i', 'j',	/*40->44, Function Keys */
	NUM, SSC, SSC,		/*45->47 */
	0xB7, SSC, SSC, 0xBA, SSC, 0xB9, SSC, SSC,	/*48->4F */
	0xB8, SSC, SSC, SSC, SSC, SSC, ALT, SSC,	/*50->57 */
};

static uint8_t state_code[] = {
	0,			/* All status are 0 */
	1,			/* SHIFT */
	0,			/* CTRL */
	1,			/* SHIFT CTRL */
	0,			/* ALT */
	1,			/* SHIFT ALT */
	3,			/* CTRL ALT */
	1,			/* SHIFT CTRL ALT */
	2,			/* CAPS */
	0,			/* CAPS SHIFT */
	2,			/* CAPS CTRL */
	0,			/* CAPS SHIFT CTRL */
	2,			/* CAPS ALT */
	0,			/* CAPS SHIFT ALT */
	2,			/* CAPS CTRL ALT */
	3,			/* CAPS SHIFT CTRL ALT */
};

static uint8_t *scan_tabs[] = {
	xtkb_scan,
	xtkb_scan_shifted,
	xtkb_scan_caps,
	xtkb_scan_ctrl_alt,
};

uint8_t keymap[16];		/* 128 codes, we don't try and deal with
				   extended keys in this for now */

/*
 *	XT style keyboard I/O is almost civilised compared
 *	with the monstrosity AT keyboards became.
 */

void xt_keyboard_irq(int irq)
{
	static uint8_t e0_prefix;
	static unsigned int kbd_state = 0;
	uint8_t E0;
	uint8_t key_up;
	uint8_t code;
	uint8_t mode;
	uint8_t E0 = 0;
	uint8_t keyp;

	code = inb_p((void *) KBD_IO);
	mode = inb_p((void *) KBD_CTL);

	/* Necessary for the XT. */
	outb_p((uint8_t) (mode | 0x80), (void *) KBD_CTL);
	outb_p((uint8_t) mode, (void *) KBD_CTL);

	/* Need to put back a raw scancode mode eventually */

	/* Remember this has been received */
	if (code == 0xE0) {
		e0_prefix = 1;
		return;
	}
	e0 = e0_prefix;
	e0_prefix = 0;

	/* On an error we cross our fingers */
	if (code == 0 || code == 0xFF)
		return;

	key_up = code & 0x80;
	code &= 0x7F;
	
	/* Track all the key up and down bits for the FUZIX raw keymap
	   interface. Probably nobody on x86 will use it but it's cheap */
	if (key_up == 0)
		key_map[code >> 3] |= 1 << (code & 7);
	else
		key_map[code >> 3] &= ~(1 << (code & 7));

	/* Work out what table to use */
	/* FIXME: check for over the end of the table ?? */
	mode = (code >= 0x1C) ? tb_state[code - 0x1C] : SSC;

	/* Process status keys */
	if (!(mode & 0xC0)) {
#if defined(CONFIG_KEYMAP_DE) || defined(CONFIG_KEYMAP_SE)
		if ((mode == ALT) && (E0 != 0))
			mode = ALT_GR;
#endif
		if (key_up)
			kbd_state ~= ~mode;
		else
			kbd_state |= mode;
		return;
	}
	if (key_up)
		return;

	switch (mode & 0xC0) {
	case 0x40:		/* F1 .. F10 */
		/* Handle Function keys  */
		code -= 0x38;
		if (kbd_mode & ALT) {
			console_switch(code);
			return;
		}
		console_queue(KEY_F1 + code);
		return;

		/* Handle extended scancodes */
	case 0x80:
		/* We have no idea how to map these so we don't */
		if (E0) {	/* Is extended scancode? */
			mode &= 0x3F;
			if (mode)
				console_queue(ESC);
			console_queue(mode + 0x0A);
			return;
		}
	}

	/* Handle CTRL-ALT-DEL  */
/*	if (code == 0x53 && (kbd_mode & CTRL) && (kbd_mode & ALT))
		ctrl_alt_del(); */

	/*
	 *      Pick the right keymap
	 */

	mode = ((kbd_mode & ~(NUM | ALT_GR)) >> 1) | (kbd_mode & 0x01);
	mode = state_code[mode];

	if (!mode && (kbd_mode & ALT_GR))
		mode = 3;
	keyp = *(scan_tabs[mode] + code);

	if ((kbd_mode & CTRL) && code < 14 && !(kbd_mode & ALT))
		keyp = xtkb_scan_shifted[code];
	if (code < 70 && (kbd_mode & NUM))
		keyp = xtkb_scan_shifted[code];
	/*
	 *      Apply special modifiers. Needs looking at more
	 */
	if ((kbd_mode & ALT && !(kbd_mode & CTRL))	/* Changed to support CTRL-ALT */
		keyp |= 0x80;	/* META-.. */
	if (!keyp)	/* non meta-@ is 64 */
		keyp = '@';
	if (kbd_mode & CTRL && !(kbd_mode & ALT))	/* Changed to support CTRL-ALT */
		keyp &= 0x1F;			/* CTRL-.. */
	if (keyp == '\r')
		keyp = '\n';
	console_queue(keyp);
}

void xt_keyboard_init(void)
{
	request_irq(1, xt_keyboard_irq);
}

#endif
