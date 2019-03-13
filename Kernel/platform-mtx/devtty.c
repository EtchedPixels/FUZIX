#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <devtty.h>
#include <vt.h>
#include <tty.h>

#undef  DEBUG			/* UNdefine to delete debug code sequences */

__sfr __at 0x0C serialAd;
__sfr __at 0x0D serialBd;
__sfr __at 0x0E serialAc;
__sfr __at 0x0F serialBc;

__sfr __at 0x09 ctc1;
__sfr __at 0x0A ctc2;

signed char vt_twidth[2] = { 80, 40 };
signed char vt_tright[2] = { 79, 39 };
uint8_t curtty;		/* output side */
uint8_t inputtty;	/* input side */
static struct vt_switch ttysave[2];

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

static tcflag_t dart_mask[4] = {
	_ISYS,
	_OSYS,
	/* FIXME CTS/RTS, CSTOPB */
	CSIZE|CBAUD|PARENB|PARODD|_CSYS,
	_LSYS,
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
	console_mask,
	dart_mask,
	dart_mask
};


/* tty1 is the screen tty2 is vdp screen */

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
	if (minor == 3)
		reg = serialAc;
	if (minor == 4)
		reg = serialBc;
	return (reg & 4) ? TTY_READY_NOW : TTY_READY_SOON;
}

void tty_putc(uint_fast8_t minor, uint_fast8_t c)
{
	irqflags_t irq;

	if (minor < 3) {
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
	/* FIXME:we can now implement flow control stuff */
}

static uint8_t dart_setup[] = {
	1, 0x19,
	2, 0x04,	/* Vector */
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

/* FIXME: Can we do CSTOPB - need to look into that */
void tty_setup(uint_fast8_t minor, uint_fast8_t flagbits)
{
	irqflags_t flags;
	int i;
	char *p = dart_setup;
	struct tty *t = &ttydata[minor];
	uint16_t cf = t->termios.c_cflag;
	uint8_t r;

	/* Console */
	if (minor < 2)
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
			r|=2;
	}
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

uint16_t keymap[8];
static uint16_t keyin[8];
static uint8_t keybyte, keybit;
static uint8_t newkey;
static int keysdown = 0;
static uint16_t shiftmask[8] = {
	0, 0, 1, 0, 1, 0, 65 , 0
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
		keyin[i] = (keyport | ((uint16_t)keyporth << 8)) ^ 0x03ff;
		key = keyin[i] ^ keymap[i];
		if (key) {
			int n;
			int m = 1;
			for (n = 0; n < 10; n++) {
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

uint8_t keyboard[8][10] = {
	{'1', '3', '5', '7', '9' , '-', '\\', KEY_PAUSE, CTRL('C'), KEY_F1},
	{ KEY_ESC, '2', '4', '6', '8', '0', '^', 0/*eol*/, KEY_BS, KEY_F5},
	{ 0/*ctrl*/, 'w', 'r', 'y', 'i', 'p', '[', KEY_UP, KEY_TAB, KEY_F2 },
	{'q', 'e', 't' , 'u', 'o', '@', KEY_ENTER, KEY_LEFT, KEY_DEL, KEY_F6 },
	{ 0/*capsl*/, 's', 'f', 'h', 'k', ';', ']', KEY_RIGHT, 0, KEY_F7 },
	{ 'a', 'd', 'g', 'j', 'l', ':', CTRL('M'), KEY_HOME, 0, KEY_F3 },
	{ 0/*shift*/, 'x', 'v', 'n', ',', '/', 0/*shift*/, KEY_DOWN, 0, KEY_F8},
	{'z', 'c', 'b', 'm', '.', '_', KEY_INSERT, KEY_CLEAR, ' ', KEY_F4 }
};

uint8_t shiftkeyboard[8][10] = {
	{'!', '#', '%', '\'', ')' , '=', '|', KEY_PAUSE, CTRL('C'), KEY_F1},
	{ KEY_ESC, '"', '$', '&', '(', 0, '~', 0/*eol*/, KEY_BS, KEY_F5},
	{ 0/*ctrl*/, 'W', 'R', 'Y', 'I', 'P', '{', KEY_UP, KEY_TAB, KEY_F2 },
	{'Q', 'E', 'T' , 'U', 'O', '`', KEY_ENTER, KEY_LEFT, KEY_DEL, KEY_F6 },
	{ 0/*capsl*/, 'S', 'F', 'H', 'K', '+', '}', KEY_RIGHT, 0, KEY_F7 },
	{ 'A', 'D', 'G', 'J', 'L', '*', CTRL('M'), KEY_HOME, 0, KEY_F3 },
	{ 0/*shift*/, 'X', 'V', 'N', '<', '/', 0/*shift*/, KEY_DOWN ,0 , KEY_F8},
	{'Z', 'C', 'B', 'M', '>', '_', KEY_INSERT, KEY_CLEAR, ' ', KEY_F4 }
};

static uint8_t capslock = 0;

static void keydecode(void)
{
	uint8_t c;

	if (keybyte == 4 && keybit == 0) {
		capslock = 1 - capslock;
		return;
	}

	if (keymap[6] & 65) {	/* shift */
		c = shiftkeyboard[keybyte][keybit];
		if (c == KEY_F1 || c == KEY_F2) {
			if (inputtty != c - KEY_F1) {
				inputtty = c - KEY_F1;
			}
			return;
		}
	} else
		c = keyboard[keybyte][keybit];



	if (keymap[2] & 1) {	/* control */
		if (c > 31 && c < 127)
			c &= 31;
	}
	if (capslock && c >= 'a' && c <= 'z')
		c -= 'a' - 'A';
	if (c)
		tty_inproc(inputtty + 1, c);
}

void kbd_interrupt(void)
{
	newkey = 0;
	keyproc();
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

/* FIXME: need to wrap vt_ioctl so we switch to the right tty before asking
   the size! */
