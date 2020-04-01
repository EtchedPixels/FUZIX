#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <devtty.h>
#include <vt.h>
#include <tty.h>
#include <graphics.h>
#include <input.h>
#include <devinput.h>
#include <mtx.h>

#undef  DEBUG			/* UNdefine to delete debug code sequences */

__sfr __at 0x0C serialAd;
__sfr __at 0x0D serialBd;
__sfr __at 0x0E serialAc;
__sfr __at 0x0F serialBc;

__sfr __at 0x09 ctc1;
__sfr __at 0x0A ctc2;

__sfr __at 0x60 prop_io;
__sfr __at 0x61 prop_rb;

signed char vt_twidth[2] = { 80, 40 };
signed char vt_tright[2] = { 79, 39 };

uint8_t curtty = 1;		/* output side */
uint8_t inputtty;		/* input side */
static struct vt_switch ttysave[2];

uint8_t prop80;			/* Propeller not a 6845 based 80 column interface */
uint8_t has_6845;

/* FIXME: this will eventually vary by tty so we'll need to either load
   it then call the vt ioctl or just intercept the vt ioctl */
uint8_t vtattr_cap;

struct vt_repeat keyrepeat;
static uint8_t kbd_timer;

static uint8_t tbuf1[TTYSIZ];
static uint8_t tbuf2[TTYSIZ];
static uint8_t tbuf3[TTYSIZ];
static uint8_t tbuf4[TTYSIZ];

static uint8_t sleeping;

struct s_queue ttyinq[NUM_DEV_TTY + 1] = {	/* ttyinq[0] is never used */
	{NULL, NULL, NULL, 0, 0, 0},
	{tbuf1, tbuf1, tbuf1, TTYSIZ, 0, TTYSIZ / 2},
	{tbuf2, tbuf2, tbuf2, TTYSIZ, 0, TTYSIZ / 2},
	{tbuf3, tbuf3, tbuf3, TTYSIZ, 0, TTYSIZ / 2},
	{tbuf4, tbuf4, tbuf4, TTYSIZ, 0, TTYSIZ / 2}
};

/*
 *	TTY masks - define which bits can be changed for each port
 */

tcflag_t termios_mask[NUM_DEV_TTY + 1] = {
	0,
	_CSYS,
	_CSYS,
	/* FIXME CTS/RTS, CSTOPB */
	CSIZE | CBAUD | PARENB | PARODD | _CSYS,
	CSIZE | CBAUD | PARENB | PARODD | _CSYS,
};


/* tty1 is the 80 column screen tty2 is vdp screen */

/* Output for the system console (kprintf etc) */
void kputchar(uint_fast8_t c)
{
	if (c == '\n')
		tty_putc(1, '\r');
	tty_putc(1, c);
}

ttyready_t tty_writeready(uint_fast8_t minor)
{
	uint8_t reg = 0xFF;
	if (minor == 1 && prop80)
		return prop_io == 255 ? TTY_READY_SOON : TTY_READY_NOW;
	if (minor == 3)
		reg = serialAc;
	if (minor == 4)
		reg = serialBc;
	return (reg & 4) ? TTY_READY_NOW : TTY_READY_SOON;
}

void tty_putc(uint_fast8_t minor, uint_fast8_t c)
{
	irqflags_t irq;

	/* The first tty might be a real 6845 80 column card but it might be
	   a propellor based one which has its own brains and video control */
	if (minor == 1 && prop80) {
		prop_io = c;
		return;
	}

	if (minor < 3) {
		/* If we have no 80 column card force everything to go via
		   the VDP. We should later do multiple VDP consoles */
		if (prop80 || has_6845) {
			/* FIXME: this makes our vt handling messy as we have the
			   IRQ off for the character I/O */
			irq = di();
			if (curtty != minor - 1) {
				vt_save(&ttysave[curtty]);
				curtty = minor - 1;
				vt_load(&ttysave[curtty]);
			}
			vtoutput(&c, 1);
			irqrestore(irq);
		} else
			vtoutput(&c, 1);
		return;
	}
	if (minor == 3)
		serialAd = c;
	else
		serialBd = c;
}

int tty_carrier(uint_fast8_t minor)
{
	uint8_t reg = 0xFF;
	if (minor == 3)
		reg = serialAc;
	if (minor == 4)
		reg = serialBc;
	return (reg & 8) ? 1 : 0;
}

void tty_sleeping(uint_fast8_t minor)
{
	sleeping |= (1 << minor);
}

void tty_data_consumed(uint_fast8_t minor)
{
	used(minor);
	/* FIXME:we can now implement flow control stuff */
}

static uint8_t dart_setup[] = {
	1, 0x19,
	2, 0x04,		/* Vector */
	3, 0x00,
	4, 0x00,
	5, 0x00,
};

/* Baud rates for Z80 DART */
static uint8_t dartbaud[] = {
	0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x80, 0x40, 0x20,
	0x10, 0x08, 0x04, 0x02, 0x01, 0x00, 0x00, 0x00
};

static uint8_t dartbits[] = {
	0x00, 0x40, 0x80, 0xC0
};

void tty_setup(uint_fast8_t minor, uint_fast8_t flagbits)
{
	irqflags_t flags;
	int i;
	char *p = dart_setup;
	struct tty *t = &ttydata[minor];
	uint16_t cf = t->termios.c_cflag;
	uint8_t r;

	used(flagbits);

	/* Console */
	if (minor <= 2)
		return;

	if ((cf & CBAUD) < B150) {
		cf &= ~CBAUD;
		cf |= B150;
	}
	if ((cf & CBAUD) > B19200) {
		cf &= ~CBAUD;
		cf |= B19200;
	}
	r = dartbits[(cf & CSIZE) >> 4];
	dart_setup[5] = 0x01 | r;
	dart_setup[9] = 0x8A | r >> 1;
	r = 0x04;
	if (cf & PARENB) {
		r |= 1;
		if (!(cf & PARODD))
			r |= 2;
	}
	if (cf & CSTOPB)
		r |= 0x08;
	dart_setup[7] = r;
	dart_setup[7] = r;

	if (minor == 3) {
		ctc1 = 0x45;
		ctc1 = dartbaud[cf & CBAUD];
	} else {
		ctc2 = 0x45;
		ctc1 = dartbaud[cf & CBAUD];
	}
	flags = di();
	for (i = 0; i < 10; i++) {
		if (minor == 3)
			serialAc = *p++;
		else
			serialBc = *p++;
	}
	irqrestore(flags);
}

/* Drop DTR/RTS on port close */
int mtxtty_close(uint_fast8_t minor)
{
	irqflags_t flags;
	int err = tty_close(minor);

	if (ttydata[minor].users)
		return 0;

	flags = di();
	if (minor == 3) {
		serialAc = 0x05;
		serialAc = 0x00;	/* Drop DTR/RTS */
	}
	if (minor == 4) {
		serialBc = 0x05;
		serialBc = 0x00;	/* Drop DTR/RTS */
	}
	irqrestore(flags);
	return err;
}

/* Stop the user opening the second console if they have no 80 column card.
   Eventually we will support multiple VDP consoles on a 16K VDP but not
   yet */
int mtxtty_open(uint_fast8_t minor, uint16_t flag)
{
	if (minor == 2 && !prop80 && !has_6845) {
		udata.u_error = ENODEV;
		return -1;
	}
	return tty_open(minor, flag);
}

uint16_t keymap[8];
static uint16_t keyin[8];
static uint8_t keybyte, keybit;
static uint8_t newkey;
static int keysdown = 0;
static uint16_t shiftmask[8] = {
	0, 0, 1, 0, 1, 0, 65, 0
};

__sfr __at 0x05 keyport;
__sfr __at 0x06 keyporth;

static void keyproc(void)
{
	int i;
	uint16_t key;

	for (i = 0; i < 8; i++) {
		/* Set the row */
		keyport = 0xff - (1 << i);
		/* Read the matrix lines - 10 bit wide */
		keyin[i] = (keyport | ((uint16_t) keyporth << 8)) ^ 0x03ff;
		key = keyin[i] ^ keymap[i];
		if (key) {
			int n;
			int m = 1;
			for (n = 0; n < 10; n++) {
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

/* TODO: non UK keyboards as identified by bits 12-11 of scan */

/* Note that we have extra entries for the 3 unused keys. These map to things
   on MTX+, PC keyboard interface module and MEMU */

uint8_t keyboard[8][10] = {
	{'1', '3', '5', '7', '9', '-', '\\', KEY_PAUSE, CTRL('C'), KEY_F1},
	{KEY_ESC, '2', '4', '6', '8', '0', '^', 0 /*eol */ , KEY_BS, KEY_F5},
	{0 /*ctrl */ , 'w', 'r', 'y', 'i', 'p', '[', KEY_UP, KEY_TAB, KEY_F2},
	{'q', 'e', 't', 'u', 'o', '@', KEY_ENTER, KEY_LEFT, KEY_DEL, KEY_F6},
	{0 /*capsl */ , 's', 'f', 'h', 'k', ';', ']', KEY_RIGHT, '=', KEY_F7},
	{'a', 'd', 'g', 'j', 'l', ':', CTRL('M'), KEY_HOME, '\'', KEY_F3},
	{0 /*shift */ , 'x', 'v', 'n', ',', '/', 0 /*shift */ , KEY_DOWN, '#', KEY_F8},
	{'z', 'c', 'b', 'm', '.', '_', KEY_INSERT, KEY_CLEAR, ' ', KEY_F4}
};

uint8_t shiftkeyboard[8][10] = {
	{'!', '#', '%', '\'', ')', '=', '|', KEY_PAUSE, CTRL('C'), KEY_F1},
	{KEY_ESC, '"', '$', '&', '(', 0, '~', 0 /*eol */ , KEY_BS, KEY_F5},
	{0 /*ctrl */ , 'W', 'R', 'Y', 'I', 'P', '{', KEY_UP, KEY_TAB, KEY_F2},
	{'Q', 'E', 'T', 'U', 'O', '`', KEY_ENTER, KEY_LEFT, KEY_DEL, KEY_F6},
	{0 /*capsl */ , 'S', 'F', 'H', 'K', '+', '}', KEY_RIGHT, '^', KEY_F7},
	{'A', 'D', 'G', 'J', 'L', '*', CTRL('M'), KEY_HOME, '@', KEY_F3},
	{0 /*shift */ , 'X', 'V', 'N', '<', '/', 0 /*shift */ , KEY_DOWN, ':', KEY_F8},
	{'Z', 'C', 'B', 'M', '>', '_', KEY_INSERT, KEY_CLEAR, ' ', KEY_F4}
};

static uint8_t capslock = 0;

static void keydecode(void)
{
	uint8_t c;
	uint8_t m = 0;

	if (keybyte == 4 && keybit == 0) {
		capslock = 1 - capslock;
		return;
	}

	if (keymap[6] & 65) {	/* shift */
		c = shiftkeyboard[keybyte][keybit];
		m = KEYPRESS_SHIFT;
		if (c == KEY_F1 || c == KEY_F2) {
			if (inputtty != c - KEY_F1) {
				inputtty = c - KEY_F1;
			}
			return;
		}
	} else
		c = keyboard[keybyte][keybit];



	if (keymap[2] & 1) {	/* control */
		m |= KEYPRESS_CTRL;
		if (c > 31 && c < 127)
			c &= 31;
	}
	if (capslock && c >= 'a' && c <= 'z')
		c -= 'a' - 'A';
	if (c) {
		switch (keyboard_grab) {
		case 0:
			vt_inproc(inputtty + 1, c);
			break;
		case 1:
			if (!input_match_meta(c)) {
				vt_inproc(inputtty + 1, c);
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

void tty_interrupt(void)
{
	uint8_t r;

	r = serialAc;
	if (r & 0x02) {
		while (r & 0x01) {
			r = serialAd;
			tty_inproc(3, r);
			r = serialAc;
		}
		if ((sleeping & 8) && (r & 0x04)) {
			sleeping &= ~8;
			tty_outproc(3);
		}
		if (!(r & 0x08))
			tty_carrier_drop(3);
		r = serialBc;
		while (r & 0x01) {
			r = serialBd;
			tty_inproc(4, r);
			r = serialBc;
		}
		if ((sleeping & 16) && (r & 0x04)) {
			sleeping &= ~16;
			tty_outproc(4);
		}
		serialAc = 0x07 << 3;	/* Return from interrupt */
		if (!(r & 0x08))
			tty_carrier_drop(4);
	}
}

/* This is used by the vt asm code, but needs to live in the kernel */
uint16_t cursorpos;

/* Need to wrap vt_ioctl so we switch to the right tty before asking
   the size! */

static struct videomap vdpmap = {
	0,
	0x01,			/* I/O ports at 1 and 2 */
	0, 0,
	0, 0,
	1,
	MAP_PIO
};

static struct display mtxmodes[3] = {
	{
		0,
		160, 96,
		80, 24,
		255, 255,
		FMT_8PIXEL_MTX,
		HW_UNACCEL,
		GFX_TEXT|GFX_MULTIMODE,
		1,
		0
	},
	{
		1,
		160, 192,
		80, 48,
		255, 255,
		FMT_8PIXEL_MTX,
		HW_UNACCEL,
		GFX_TEXT|GFX_MULTIMODE,
		1,
		0
	},
		/* VDP: we need to think harder about how we deal with VDP mode
		   setting here and in MSX TODO */
	{
		1,
		256, 192,
		256, 192,
		255, 255,
		FMT_VDP,
		HW_VDP_9918A,
		GFX_MULTIMODE | GFX_MAPPABLE | GFX_TEXT,
		16,
		0
	},
};

static uint8_t vmode[3] = {
	0, /* Unused */
	0, /* 80x24 text */
	2, /* VDP */
};

__sfr __at 0x38 crtc_reg;
__sfr __at 0x39 crtc_data;

/*
 *	TODO: VDP font setting and UDG. Also the same is needed for the prop80
 *	with it's 20x8 programmable characters in weird format.
 */
int mtx_vt_ioctl(uint_fast8_t minor, uarg_t request, char *data)
{
	uint8_t dev = minor;
	if (minor > 2)
		return tty_ioctl(minor, request, data);

	/* If we are 40 column only then our graphics properties change */
	if (!prop80 && !has_6845)
		dev = 2;

	if (request == GFXIOC_GETINFO)
		return uput(&mtxmodes[vmode[minor]], data, sizeof(struct display));
	if (request == GFXIOC_MAP && dev == 2)
		return uput(&vdpmap, data, sizeof(struct videomap));
	if (request == GFXIOC_UNMAP)
		return 0;
	if (request == GFXIOC_GETMODE || request == GFXIOC_SETMODE)
	{
		uint8_t m = ugetc(data);
		if (m > 1 || dev > 1 || (m == 1 && !has_rememo)) {
			udata.u_error = EINVAL;
			return -1;
		}
		if (request == GFXIOC_GETMODE)
			return uput(&mtxmodes[m], data, sizeof(struct display));
		else {
			irqflags_t irq;
			irq = di();
			/* Set CRT register 31 */
			crtc_reg = 31;
			crtc_data = m;
			irqrestore(irq);
		}
		return 0;
	}
	if (request == VTSIZE) {
		if (dev == 1) {
			if (vmode[1] == 0)
				return (24 << 8) | 80;
			return (48 << 8) | 80;
		}
		if (dev == 2)
			return (24 << 8) | 40;
	}
	return vt_ioctl(minor, request, data);
}

__sfr __at 0x30 bingbong;

void do_beep(void)
{
	volatile uint8_t unused;
	if (has_6845)
		unused = bingbong;
}

/*
 *	See if our 80 column card is a propellor board or a 6845 based board
 *
 *	Must be called very early so we drive the right screen!
 */
static int prop_wait(void)
{
	uint16_t i;
	for (i = 0; i < 8192; i++) {
		if (prop_io == 0)
			return 0;
	}
	return 1;
}

int probe_prop(void)
{
	if (prop_wait())
		return 0;
#if 0
	prop_io = 0x1B;
	prop_io = 0x9C;
	prop_rb = 0xB4;
	if (prop_wait() || prop_rb)
		return 0;
	prop_io = 0x1B;
	prop_io = 0x9E;
	prop_rb = 0xB4;
	if (prop_wait() || prop_rb != 2)
		return 0;
#endif
	prop80 = 1;
	curtty = 0;
	return 1;
}
