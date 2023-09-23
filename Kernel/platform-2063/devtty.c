#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <vt.h>
#include <tty.h>
#include <graphics.h>
#include <vdp1.h>
#include <devtty.h>
#include <2063.h>

static char tbuf1[TTYSIZ];
static char tbuf2[TTYSIZ];

/* We don't support console switching yet (no real keyboard) but leave the
   framework in place */
static uint8_t tmsinkpaper[5] = { 0, 0xF4 };
static uint8_t tmsborder[5]  = {0, 0x04 };
uint8_t vtattr_cap;
uint8_t vidmode;
uint8_t inputtty = 1;
uint8_t outputtty = 1;
static uint8_t vswitch;
int8_t vt_tright = 39;
int8_t vt_twidth = 40;
uint16_t vdpport = 0x2881;	/* 40 chars wide port 0x81 */

static uint8_t sleeping;

struct s_queue ttyinq[NUM_DEV_TTY + 1] = {	/* ttyinq[0] is never used */
	{NULL, NULL, NULL, 0, 0, 0},
	{tbuf1, tbuf1, tbuf1, TTYSIZ, 0, TTYSIZ / 2},
	{tbuf2, tbuf2, tbuf2, TTYSIZ, 0, TTYSIZ / 2},
};

tcflag_t termios_mask[NUM_DEV_TTY + 1] = {
	0,
	CSIZE|CSTOPB|PARENB|PARODD|_CSYS,
	CSIZE|CSTOPB|PARENB|PARODD|_CSYS
};

uint8_t sio_r[] = {
	0x03, 0xC1,
	0x04, 0x44,
	0x05, 0xEA
};

static const uint8_t sio2_cmap[3] = {
	0x00,	/* unused */
	0x32,
	0x33,
};

static const uint8_t sio_dmap[3] = {
	0x00,	/* unused */
	0x30,
	0x31,
};

/* We are clocking at 1.8432MHz and x16 or x64 for the low speeds */
static const uint16_t siobaud[] = {
	0xC0,	/* 0 */
	0,	/* 50 */
	0,	/* 75 */
	0,	/* 110 */
	0xD6,	/* 134 */
	0xC0,	/* 150 */
	0x60,	/* 300 */
	0xC0,	/* 600 */
	0x60,	/* 1200 */
	0x30,	/* 2400 */
	0x18,	/* 4800 */
	0x0C,	/* 9600 */
	0x06,	/* 19200 */
	0x03,	/* 38400 */
	0x02,	/* 57600 */
	0x01	/* 115200 */
};

static void sio2_setup(uint8_t minor, uint8_t flags)
{
	struct termios *t = &ttydata[minor].termios;
	uint8_t r;
	uint8_t baud;

	baud = t->c_cflag & CBAUD;
	if (baud < B134)
		baud = B134;

	/* Set bits per character */
	sio_r[1] = 0x01 | ((t->c_cflag & CSIZE) << 2);

	r = 0xC4;

	if (minor == 1) {
		CTC_CH1 = 0x55;
		CTC_CH1 = siobaud[baud];
	} else {
		CTC_CH2 = 0x55;
		CTC_CH2 = siobaud[baud];
	}
	if (baud >= B600)	/* Use x16 clock and CTC divider */
		r = 0x44;

	t->c_cflag &= ~CBAUD;
	t->c_cflag |= baud;
	if (t->c_cflag & CSTOPB)
		r |= 0x08;
	if (t->c_cflag & PARENB)
		r |= 0x01;
	if (t->c_cflag & PARODD)
		r |= 0x02;
	sio_r[3] = r;
	sio_r[5] = 0x8A | ((t->c_cflag & CSIZE) << 1);
}

void tty_setup(uint8_t minor, uint8_t flags)
{
	sio2_setup(minor, flags);
	sio2_otir(sio2_cmap[minor]);
}

int tty_carrier(uint8_t minor)
{
        uint8_t c;
        uint8_t port;

	port = sio2_cmap[minor];
	out(port, 0);
	c = in(port);
	if (c & 0x08)
		return 1;
	return 0;
}

void tty_drain_sio(void)
{
	static uint8_t old_ca[2];

	while(sio_rxl[0])
		tty_inproc(1, sioa_rx_get());
	if (sio_dropdcd[0]) {
		sio_dropdcd[0] = 0;
		tty_carrier_drop(1);
	}
	if (((old_ca[0] ^ sio_state[0]) & sio_state[0]) & 8)
		tty_carrier_raise(1);
	old_ca[0] = sio_state[0];
	if (sio_txl[0] < 64 && (sleeping & (1 << 1))) {
		sleeping &= ~(1 << 1);
		tty_outproc(1);
	}

	while(sio_rxl[1])
		tty_inproc(2, siob_rx_get());
	if (sio_dropdcd[1]) {
		sio_dropdcd[1] = 0;
		tty_carrier_drop(2);
	}
	if (((old_ca[1] ^ sio_state[1]) & sio_state[1]) & 8)
		tty_carrier_raise(2);
	old_ca[1] = sio_state[1];
	if (sio_txl[1] < 64 && (sleeping & (1 << 2))) {
		sleeping &= ~(1 << 2);
		tty_outproc(2);
	}
}

void tty_putc(uint8_t minor, unsigned char c)
{
	irqflags_t irqflags = di();

	switch(minor) {
	case 1:
		if (minor == 1 && !vswitch && vdptype != 0xFF)
			vtoutput(&c, 1);
		sioa_txqueue(c);
		break;
	case 2:
		siob_txqueue(c);
		break;
	}
	irqrestore(irqflags);
}

void tty_sleeping(uint8_t minor)
{
	sleeping |= (1 << minor);
}

ttyready_t tty_writeready(uint8_t minor)
{
	if (sio_txl[minor - 1] < 128)
		return TTY_READY_NOW;
	return TTY_READY_SOON;
}

void tty_data_consumed(uint8_t minor)
{
	used(minor);
}

/* kernel writes to system console -- never sleep! */

void kputchar(char c)
{
	/* Can't use the normal paths as we must survive interrupts off */
	irqflags_t irq = di();

	while(!(SIOA_C & 0x04));
	SIOA_D = c;
	if (vdptype != 0xFF)
		vtoutput(&c, 1);
	if (c == '\n')
		kputchar('\r');

	irqrestore(irq);
}

/* VDP glue */

/* This is used by the vt asm code, but needs to live in the kernel */
uint16_t cursorpos;

/*
 *	TTY glue
 */

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

void vdp_reload(void)
{
	irqflags_t irq = di();

	if (vidmode == 0) {
		vdp_setup40();
		vt_twidth = 40;
		vt_tright = 39;
	} else {
		vdp_setup32();
		vt_twidth = 32;
		vt_tright = 31;
	}
	vdp_restore_font();
	vdp_udgload();
	vt_cursor_off();
	vt_cursor_on();
	vdp_attributes();
	irqrestore(irq);
}

/*
 *	Graphics layer interface (for TMS9918A and friends)
 */

static struct videomap tms_map = {
	0,
	0x80,
	0, 0,
	0, 0,
	1,
	MAP_PIO
};

/*
 *	We always report a TMS9918A. Now it is possible that someone
 *	is using a 9938 or 9958 so we might want to add some VDP autodetect
 *	code and report accordingly but that's a minor TODO
 */
static struct display tms_mode[2] = {
	{
		0,
		256, 192,
		256, 192,
		255, 255,
		FMT_VDP,
		HW_VDP_9918A,
		GFX_MULTIMODE|GFX_MAPPABLE|GFX_TEXT|GFX_VBLANK,
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
		GFX_MULTIMODE|GFX_MAPPABLE|GFX_TEXT|GFX_VBLANK,
		16,
		0,
		32, 24
	}
};

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


int vdptty_ioctl(uint8_t minor, uarg_t arg, char *ptr)
{
  unsigned i = 0;
  uint_fast8_t is_wr = 0;
  unsigned topchar = 256;
  uint8_t c;
  uint8_t map[8];

  if (minor == 1 && vdptype != 0xFF) {
	switch(arg) {
	case GFXIOC_GETINFO:
                return uput(&tms_mode[vidmode], ptr, sizeof(struct display));
	case GFXIOC_MAP:
		if (vswitch) {
			udata.u_error = EBUSY;
			return -1;
		}
		vswitch = minor;
		return uput(&tms_map, ptr, sizeof(struct videomap));
	case GFXIOC_UNMAP:
		if (vswitch == minor) {
			vdp_reload();
			vswitch = 0;
		}
		return 0;
	case GFXIOC_GETMODE:
	case GFXIOC_SETMODE: {
		uint8_t m = ugetc(ptr);
		if (m > 1) {
			udata.u_error = EINVAL;
			return -1;
		}
		if (arg == GFXIOC_GETMODE)
			return uput(&tms_mode[m], ptr, sizeof(struct display));
		vidmode = m;
		vdp_reload();
		return 0;
		}
	case GFXIOC_WAITVB:
		psleep(&vdpport);
		return 0;
	case VDPIOC_SETUP:
		/* Must be locked to issue */
		if (vswitch == 0) {
			udata.u_error = EINVAL;
			return -1;
		}
		if (uget(ptr, map, 8) == -1)
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
		if (uget(ptr, &rw, sizeof(struct vdp_rw)))
			return -1;
		addr = (uint8_t *)rw.data;
		size = rw.lines * rw.cols;
		if (valaddr(addr, size, is_wr) != size) {
			udata.u_error = EFAULT;
			return -1;
		}
		if (arg == VDPIOC_READ)
			udata.u_error = vdp_rop(&rw);
		else
			udata.u_error = vdp_wop(&rw);
		if (udata.u_error)
			return -1;
		return 0;
	}
	case VTBORDER:
		c = ugetc(ptr);
		tmsborder[inputtty]  = igrb_to_msx(c & 0x1F);
		vdp_attributes();
		return 0;
	case VTINK:
		c = ugetc(ptr);
		tmsinkpaper[inputtty] &= 0x0F;
		tmsinkpaper[inputtty] |= igrb_to_msx(c & 0x1F) << 4;
		vdp_attributes();
		return 0;
	case VTPAPER:
		c = ugetc(ptr);
		tmsinkpaper[inputtty] &= 0xF0;
		tmsinkpaper[inputtty] |= igrb_to_msx(c & 0x1F);
		vdp_attributes();
		return 0;
	case VTFONTINFO:
		return uput(fonti + vidmode, ptr, sizeof(struct fontinfo));
	case VTSETUDG:
		i = ugetc(ptr);
		ptr++;
		if (i < 128 || i >= 159) {
			udata.u_error = EINVAL;
			return -1;
		}
		topchar = i + 1;
	case VTSETFONT:
		while(i < topchar) {
			if (uget(ptr, map, 8) == -1)
				return -1;
			ptr += 8;
			vdp_set_char(i++, map);
		}
		vdp_udgsave();
		vdp_udgload();
		return 0;
	}
	return vt_ioctl(minor, arg, ptr);
  }
  /* Not a VT port so don't go via generic VT */
  return tty_ioctl(minor, arg, ptr);
}

int vdptty_close(uint_fast8_t minor)
{
	if (vswitch == minor) {
		vdp_reload();
		vswitch = 0;
	}
	return tty_close(minor);
}

void do_beep(void)
{
}
