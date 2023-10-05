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
#include <vdp1.h>

/*
 *	Our tty ports are
 *
 *	0:	always logical /dev/tty - never seen by us
 *	1-4:	VDP Consoles
 *	5:	May be an 80 column card
 *	6-7: 	serial ports
 */

#undef  DEBUG			/* Undefine to delete debug code sequences */

#define TTY_80COL	5
#define TTY_SERA	6
#define TTY_SERB	7

__sfr __at 0x0C serialAd;
__sfr __at 0x0D serialBd;
__sfr __at 0x0E serialAc;
__sfr __at 0x0F serialBc;

__sfr __at 0x09 ctc1;
__sfr __at 0x0A ctc2;

__sfr __at 0x60 prop_io;
__sfr __at 0x61 prop_rb;

signed char vt_twidth[6] = { 0, 40, 40, 40, 40, 80 };
signed char vt_tright[6] = { 0, 39, 39, 39, 39, 79 };
static uint8_t vmode[6];	/* mode of each console */
static uint8_t tmsinkpaper[5] = { 0, 0xF4 };
static uint8_t tmsborder[5]  = {0, 0x04 };
uint8_t vidmode;		/* mode of the moment */

uint8_t inputtty = 1;		/* input side */
uint8_t outputtty;		/* output side */
static uint8_t vswitch;
static uint8_t syscon = 1;	/* system console output */

static uint16_t tty_present = 0xDF;	/* 2 serial, may be gap, 4 consoles, unused bit */
static struct vt_switch ttysave[6];	/* 0 unused */

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
static uint8_t tbuf5[TTYSIZ];
static uint8_t tbuf6[TTYSIZ];
static uint8_t tbuf7[TTYSIZ];

static uint8_t sleeping;

struct s_queue ttyinq[NUM_DEV_TTY + 1] = {	/* ttyinq[0] is never used */
	{NULL, NULL, NULL, 0, 0, 0},
	{tbuf1, tbuf1, tbuf1, TTYSIZ, 0, TTYSIZ / 2},
	{tbuf2, tbuf2, tbuf2, TTYSIZ, 0, TTYSIZ / 2},
	{tbuf3, tbuf3, tbuf3, TTYSIZ, 0, TTYSIZ / 2},
	{tbuf4, tbuf4, tbuf4, TTYSIZ, 0, TTYSIZ / 2},
	{tbuf5, tbuf3, tbuf3, TTYSIZ, 0, TTYSIZ / 2},
	{tbuf6, tbuf4, tbuf4, TTYSIZ, 0, TTYSIZ / 2},
	{tbuf7, tbuf3, tbuf3, TTYSIZ, 0, TTYSIZ / 2},
};

/*
 *	TTY masks - define which bits can be changed for each port
 */

tcflag_t termios_mask[NUM_DEV_TTY + 1] = {
	0,
	_CSYS,
	_CSYS,
	_CSYS,
	_CSYS,
	_CSYS,
	/* FIXME CTS/RTS, CSTOPB */
	CSIZE | CBAUD | PARENB | PARODD | _CSYS,
	CSIZE | CBAUD | PARENB | PARODD | _CSYS,
};

/*
 *	Switch output device
 */
static void set_output(uint_fast8_t minor)
{
	outputtty = minor;
}

/*
 *	Switch input device
 */
static void set_input(uint_fast8_t minor)
{
	minor--;
	if (vswitch)
		return;
	if (!(tty_present & (1 << minor)))
		return;
	inputtty = minor;
	/* 80 column lives on its own monitor */
	if (inputtty != TTY_80COL)
		vdp_set_console();
}

static void vdp_writeb(uint16_t addr, uint8_t v)
{
	vdp_set(addr|0x4000);
	vdp_out(v);
}

static void vdp_set_char(uint_fast8_t c, uint8_t *d)
{
	irqflags_t irq = di();
	unsigned addr = 0x1000 + 8 * c;
	uint_fast8_t i;
	for (i = 0; i < 8; i++)
		vdp_writeb(addr++, *d++);
	irqrestore(irq);
}

static void vdp_udgsave(void)
{
	irqflags_t irq = di();
	unsigned i;
	unsigned uaddr = 0x1400;	/* Char 128-159 */
	unsigned addr = 0x3800 + 256 * (inputtty - 1);
	for (i = 0; i < 256; i++)
		vdp_writeb(addr, vdp_readb(uaddr++));
	irqrestore(irq);
}

static void vdp_udgload(void)
{
	irqflags_t irq = di();
	unsigned i;
	unsigned addr = 0x3800 + 256 * (inputtty - 1);
	unsigned uaddr = 0x1000;		/* Char 128-159, inverses at 0-31 for cursor */
	for (i = 0; i < 256; i++) {
		uint8_t c = vdp_readb(uaddr++);
		vdp_writeb(addr, ~c);
		vdp_writeb(addr + 0x400, c);
	}
	irqrestore(irq);
}

/* Restore colour attributes */
void vdp_attributes(void)
{
	irqflags_t irq = di();
	if (vidmode == 1) {
		vdp_setcolour(tmsinkpaper[inputtty]);
		vdp_setborder(tmsborder[inputtty]);
	} else {
		vdp_setborder(tmsinkpaper[inputtty]);
	}
	irqrestore(irq);
}


/*
 *	VDP mode set up
 */

static void vdp_restore(void)
{
	irqflags_t irq = di();
	uint_fast8_t minor = inputtty;

	vidmode = vmode[minor];
	if (vidmode) {
		vt_twidth[minor] = 32;
		vt_tright[minor] = 31;
		vdp_setup32();
	} else {
		vt_twidth[minor] = 40;
		vt_tright[minor] = 39;
		vdp_setup40();
	}
	vdp_restore_font();
	vdp_udgload();
	vt_cursor_off();
	vt_cursor_on();
	vdp_attributes();
	irqrestore(irq);
}

/* Output for the system console (kprintf etc) */
void kputchar(uint_fast8_t c)
{
	if (c == '\n')
		tty_putc(syscon, '\r');
	tty_putc(syscon, c);
}

ttyready_t tty_writeready(uint_fast8_t minor)
{
	uint8_t reg = 0xFF;
	if (minor == TTY_80COL && prop80)
		return prop_io == 255 ? TTY_READY_SOON : TTY_READY_NOW;
	if (minor == TTY_SERA)
		reg = serialAc;
	if (minor == TTY_SERB)
		reg = serialBc;
	return (reg & 4) ? TTY_READY_NOW : TTY_READY_SOON;
}

void tty_putc(uint_fast8_t minor, uint_fast8_t c)
{
	irqflags_t irq;

	/* The first tty might be a real 6845 80 column card but it might be
	   a propellor based one which has its own brains and video control */
	if (minor == TTY_80COL && prop80) {
		prop_io = c;
		return;
	}

	if (minor < TTY_SERA) {
		if (vswitch)
			return;
		/* Need a 'defer conswitch' to clean up the IRQ handling */
		irq = di();
		if (outputtty != minor) {
			vt_save(&ttysave[outputtty]);
			set_output(minor);
			vt_load(&ttysave[outputtty]);
		}
		vtoutput(&c, 1);
		irqrestore(irq);
		return;
	}
	if (minor == TTY_SERA)
		serialAd = c;
	else
		serialBd = c;
}

int tty_carrier(uint_fast8_t minor)
{
	uint8_t reg = 0xFF;
	if (minor == TTY_SERA)
		reg = serialAc;
	if (minor == TTY_SERB)
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
	if (minor < TTY_SERA)
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

	if (minor == vswitch)
		vdp_restore();
	flags = di();
	if (minor == TTY_SERA) {
		serialAc = 0x05;
		serialAc = 0x00;	/* Drop DTR/RTS */
	}
	if (minor == TTY_SERB) {
		serialBc = 0x05;
		serialBc = 0x00;	/* Drop DTR/RTS */
	}
	irqrestore(flags);
	return err;
}

/*
 *	Stop opening any non-existant hardware.
 */
int mtxtty_open(uint_fast8_t minor, uint16_t flag)
{
	if (tty_present & (1 << minor))
		return tty_open(minor, flag);
	udata.u_error = ENODEV;
	return -1;
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
		/* FIXME: TODO */
		if (c >= KEY_F1 &&  c <= KEY_F5) {
			if (inputtty != c - KEY_F1)
				set_input(c - KEY_F1);
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
			vt_inproc(inputtty, c);
			break;
		case 1:
			if (!input_match_meta(c)) {
				vt_inproc(inputtty, c);
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
			tty_inproc(TTY_SERA, r);
			r = serialAc;
		}
		if ((sleeping & 8) && (r & 0x04)) {
			sleeping &= ~8;
			tty_outproc(TTY_SERA);
		}
		if (!(r & 0x08))
			tty_carrier_drop(TTY_SERA);
		r = serialBc;
		while (r & 0x01) {
			r = serialBd;
			tty_inproc(TTY_SERB, r);
			r = serialBc;
		}
		if ((sleeping & 16) && (r & 0x04)) {
			sleeping &= ~16;
			tty_outproc(TTY_SERB);
		}
		serialAc = 0x07 << 3;	/* Return from interrupt */
		if (!(r & 0x08))
			tty_carrier_drop(TTY_SERB);
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

/*
 *	Display modes for the 6845 style video
 */
static struct display mtx_mode = {
		0,
		160, 96,
		80, 24,
		255, 255,
		FMT_8PIXEL_MTX,
		HW_UNACCEL,
		GFX_TEXT|GFX_MULTIMODE,
		1,
		0,
		80, 24
};

/*
 *	We always report a TMS9918A. Now it is possible that someone
 *	is using a 9938 or 9958 so we might want to add some VDP autodetect
 *	code and report accordingly but that's a minor TODO
 */
static struct display vdp_mode[2] = {
	{
		0,
		256, 192,
		256, 192,
		255, 255,
		FMT_VDP,
		HW_VDP_9918A,
		GFX_MULTIMODE|GFX_MAPPABLE|GFX_TEXT,
		16,
		0,
		40, 24
	},
	{
		1,
		256, 192,
		256, 192,
		255, 255,
		FMT_VDP,
		HW_VDP_9918A,
		GFX_MULTIMODE|GFX_MAPPABLE|GFX_TEXT,
		16,
		0,
		32, 24
	}
};


static uint8_t vmode[6] = {
	0, /* Unused */
	0, /* VDP 40 column */
	0, /* VDP 40 column */
	0, /* VDP 40 column */
	0, /* VDP 40 column */
	0  /* 80 column card */
};

__sfr __at 0x38 crtc_reg;
__sfr __at 0x39 crtc_data;

static struct fontinfo fonti[] = {
	{ 0, 255, 128, 159, FONT_INFO_6X8 },
	{ 0, 255, 128, 159, FONT_INFO_8X8 },
};

static uint8_t igrbmsx[16] = {
	1,	/* 0000 to Black */
	4,	/* 000B to 4 dark blue */
	6,	/* 00R0 to 6 dark red */
	13,	/* 00RB to magneta */
	12,	/* 0G00 to dark green */
	7,	/* 0G0B to cyan */
	10,	/* 0GR0 to dark yellow */
	14,	/* 0GRB to grey */
	14,	/* I000 to grey */
	5,	/* I00B to light blue */
	9,	/* 10R0 to light red */
	8,	/* 10RB to magenta or medium red ? - use mr for now */
	3,	/* 1G00 to light green */
	7,	/* 1G0B to cyan - no light cyan */
	11,	/* 1GR0 to light yellow */
	15	/* 1GRB to white */
};

static uint8_t igrb_to_msx(uint8_t c)
{
	/* Machine specific colours */
	if (c & 0x10)
		return c & 0x0F;
	/* IGRB colours */
	return igrbmsx[c & 0x0F];
}

/*
 *	TODO: VDP font setting and UDG. Also the same is needed for the prop80
 *	with it's 20x8 programmable characters in weird format.
 */

static int mtx_vt_ioctl80(uint_fast8_t minor, uarg_t request, char *data)
{
	if (request == GFXIOC_GETINFO)
		return uput(&mtx_mode, data, sizeof(struct display));
	if (request == VTSIZE)
		return (24 << 8) | 80;
	return vt_ioctl(minor, request, data);
}

int mtx_vt_ioctl(uint_fast8_t minor, uarg_t request, char *data)
{
	uint8_t dev = minor;
	uint8_t is_wr = 0;
	uint8_t map[8];
	uint8_t c;
	unsigned i = 0;
	unsigned topchar = 255;

	if (minor >= TTY_SERA)
		return tty_ioctl(minor, request, data);

	/* If we are 40 column only then our graphics properties change */
	if (minor == TTY_80COL)
		return mtx_vt_ioctl80(minor, request, data);

	switch(request) {
	case GFXIOC_GETINFO:
		return uput(&vdp_mode[vmode[minor]], data, sizeof(struct display));
	case GFXIOC_MAP:
		if (vswitch) {
			udata.u_error = EBUSY;
			return -1;
		}
		vswitch = minor;
		return uput(&vdpmap, data, sizeof(struct videomap));
	case GFXIOC_UNMAP:
		return 0;
	case GFXIOC_GETMODE:
	case GFXIOC_SETMODE:
	{
		uint8_t m = ugetc(data);
		if (m > 1) {
			udata.u_error = EINVAL;
			return -1;
		}
		if (request == GFXIOC_GETMODE)
			return uput(&vdp_mode[m], data, sizeof(struct display));
		else {
			vmode[minor] = m;
			vdp_restore();
		}
		return 0;
	}
	case VDPIOC_SETUP:
		/* Must be locked to issue */
		if (vswitch == 0) {
			udata.u_error = EINVAL;
			return -1;
		}
		if (uget(data, map, 8) == -1)
			return -1;
		map[1] |= 0xA0;
		vdp_setup(map);
		return 0;
	case VDPIOC_READ:
		is_wr = 1;
	case VDPIOC_WRITE:
	{
		struct vdp_rw rw;
		uint16_t size;
		uint8_t *addr;
		if (vswitch == 0) {
			udata.u_error = EINVAL;
			return -1;
		}
		if (uget(data, &rw, sizeof(struct vdp_rw)))
			return -1;
		addr = (uint8_t *)rw.data;
		size = rw.lines * rw.cols;
		if (valaddr(addr, size, is_wr) != size) {
			udata.u_error = EFAULT;
			return -1;
		}
		if (request == VDPIOC_READ)
			udata.u_error = vdp_rop(&rw);
		else
			udata.u_error = vdp_wop(&rw);
		if (udata.u_error)
			return -1;
		return 0;
	}
	case VTBORDER:
		c = ugetc(data);
		tmsborder[inputtty]  = igrb_to_msx(c & 0x1F);
		vdp_attributes();
		return 0;
	case VTINK:
		c = ugetc(data);
		tmsinkpaper[inputtty] &= 0x0F;
		tmsinkpaper[inputtty] |= igrb_to_msx(c & 0x1F) << 4;
		vdp_attributes();
		return 0;
	case VTPAPER:
		c = ugetc(data);
		tmsinkpaper[inputtty] &= 0xF0;
		tmsinkpaper[inputtty] |= igrb_to_msx(c & 0x1F);
		vdp_attributes();
		return 0;
	case VTFONTINFO:
		return uput(fonti + vmode[minor], data, sizeof(struct fontinfo));
	case VTSETUDG:
		i = ugetc(data);
		data++;
		if (i < 128 || i >= 159) {
			udata.u_error = EINVAL;
			return -1;
		}
		topchar = i + 1;
	case VTSETFONT:
		while(i < topchar) {
			if (uget(data, map, 8) == -1)
				return -1;
			data += 8;
			vdp_set_char(i++, map);
		}
		vdp_udgsave();
		vdp_udgload();
		return 0;
	case VTSIZE:
		return (24 << 8) | vt_twidth[minor];
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
	syscon = TTY_80COL;
	tty_present |= (1 << TTY_80COL);
	return 1;
}
