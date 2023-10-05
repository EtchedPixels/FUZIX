/*
 *	TMS9918A support code. Note the probe code is in discard and
 *	the low level rendering is in vdp1.s
 */

#include <kernel.h>
#include <kdata.h>
#include <tty.h>
#include <graphics.h>
#include <devtty.h>
#include <rcbus.h>
#include <vt.h>
#include <cpu_ioctl.h>
#include <softzx81.h>
#include "multivt.h"
/*
 *	Graphics layer interface (for TMS9918A and friends)
 */

static uint8_t mode[5];		/* Per console 1-4 */
static uint8_t tmsinkpaper[5] = { 0, 0xF4, 0xF4, 0xF4, 0xF4 };
static uint8_t tmsborder[5] = { 0, 0x04, 0x04, 0x04, 0x04 };

static uint8_t tms_width;
uint8_t vidmode;		/* For live screen */

static struct videomap tms_map = {
	0,
	0x98,
	0, 0,
	0, 0,
	1,
	MAP_PIO
};


/* FIXME: we need a way of reporting CPU speed/TMS delay info as unlike the
   ports so far we need delays on the RC2014 */

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
	 GFX_MULTIMODE | GFX_MAPPABLE | GFX_TEXT | GFX_VBLANK,
	 16,
	 0,
	 40, 24 },
	{
	 1,
	 256, 192,
	 256, 192,
	 255, 255,
	 FMT_VDP,
	 HW_VDP_9918A,
	 GFX_MULTIMODE | GFX_MAPPABLE | GFX_TEXT | GFX_VBLANK,
	 16,
	 0,
	 32, 24 }
};


/*
 *	TMS9918A configuration
 *	Text mode is wired into vdp1.s
 *
 *	0x3C00-0x3FFF are reserved by the OS.
 *
 *	The text mode configuration we use is
 *	Secret font store at 3C00-3FFF (128 base symbols copy)
 *	Secret UDG stash at 3800-3BFF in future maybe (32 chars x 4 so 1K)
 *	4 screens base 0x0000 + 0x400 per screen
 *	Patterns base 0x1000
 *
 *	For graphics:
 *		Sprite Patterns 0x1800
 *		Colour table 0x2000
 *		Sprite attribute 3B00-3BFF
 *	For graphics two non-standard we can maybe do similar ?
 */

static const uint8_t tmsreset[8] = {
	0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static const uint8_t tmstext[8] = {
	0x00, 0xD0, 0x00, 0x00, 0x02, 0x00, 0x00, 0xF4	/* Text, no IRQ */
};

static const uint8_t tmstext32[8] = {
	0x00, 0xC2, 0x00, 0x80, 0x02, 0x76, 0x03, 0x04	/* Text, no IRQ */
};

/* Should move these helpers into asm TODO */
static void nap(void)
{
}

void tms9918a_config(const uint8_t * r)
{
	irqflags_t irq = di();
	uint8_t c = 0x80;
	while (c < 0x88) {
		tms9918a_ctrl = *r++;
		tms9918a_ctrl = c++;
		nap();
	}
	irqrestore(irq);
}

static uint8_t tms_readb(uint16_t addr)
{
	tms9918a_ctrl = addr;
	tms9918a_ctrl = addr >> 8;
	nap();
	return tms9918a_data;
}

static void tms_writeb(uint16_t addr, uint8_t data)
{
	tms9918a_ctrl = addr;
	tms9918a_ctrl = (addr >> 8) | 0x40;
	tms9918a_data = data;
}

/* Reset the TMS9918A and turn off its interrupt */
void tms9918a_reset(void)
{
	tms9918a_config(tmsreset);
}

void tms9918a_set_char(uint_fast8_t c, uint8_t * d)
{
	irqflags_t irq = di();
	unsigned addr = 0x1000 + 8 * c;
	uint_fast8_t i;
	for (i = 0; i < 8; i++)
		tms_writeb(addr++, *d++);
	irqrestore(irq);
}

void tms9918a_udgsave(void)
{
	irqflags_t irq = di();
	unsigned i;
	unsigned uaddr = 0x1400;	/* Char 128-159 */
	unsigned addr = 0x3800 + 256 * (inputtty - 1);
	for (i = 0; i < 256; i++)
		tms_writeb(addr, tms_readb(uaddr++));
	irqrestore(irq);
}

void tms9918a_udgload(void)
{
	irqflags_t irq = di();
	unsigned i;
	unsigned addr = 0x3800 + 256 * (inputtty - 1);
	unsigned uaddr = 0x1000;	/* Char 128-159, inverses at 0-31 for cursor */
	for (i = 0; i < 256; i++) {
		uint8_t c = tms_readb(uaddr++);
		tms_writeb(addr, ~c);
		tms_writeb(addr + 0x400, c);
	}
	irqrestore(irq);
}

/* Restore colour attributes */
void tms9918a_attributes(void)
{
	irqflags_t irq = di();
	if (mode[inputtty]) {
		unsigned addr;
		uint8_t c = tmsinkpaper[inputtty];
		addr = 0x2000;
		while (addr != 0x2020)
			tms_writeb(addr++, c);
		tms9918a_ctrl = tmsborder[inputtty];
		tms9918a_ctrl = 0x87;
	} else {
		tms9918a_ctrl = tmsinkpaper[inputtty];
		tms9918a_ctrl = 0x87;
	}
	irqrestore(irq);
}


void tms9918a_attributes_m(uint8_t minor)
{
	if (inputtty == minor)
		tms9918a_attributes();
}

struct tmsinfo {
	uint16_t lastline;
	uint16_t dmov;
	uint16_t s1;
	uint16_t w;
	uint16_t umov;
	uint8_t *conf;
	uint8_t inton;
};

static struct tmsinfo tmsdat[2] = {
	{
		0x03C0,
		0xFFD8,
		0x4028,
		0x0028,
		0x3FD8,
		tmstext,
		0xF0
	}, {
		0x02E0,
		0xFFE0,
		0x4020,
		0x0020,
		0x3FE0,
		tmstext32,
		0xE2
	}
};

/* This relies on the TMS9918A interrupt being off */
void tms9918a_reload(void)
{
	uint16_t r;
	uint8_t b;
	struct tmsinfo *t;

	vidmode = mode[inputtty];
	t = tmsdat + vidmode;

	for (r = 0; r < 0x400; r++) {
		b = tms_readb(0x3C00 + r);
		tms_writeb(0x1000 + r, b);
		tms_writeb(0x1400 + r, ~b);
	}
	tms_writeb(0x3B00, 0xD0);
	tms9918a_config(t->conf);
	tms_width = t->w;
	/* Set up the scrollers in the asm side */
	scrolld_base = t->lastline;	/* Start of last line */
	scrolld_mov = t->dmov;	/* How to move up */
	scrolld_s1 = t->s1;	/* Move back and turn on write */
	scrollu_w = t->w;	/* Start upscroll on line 1 */
	scrollu_mov = t->umov;	/* Move up 40 bytes, and add 0x4000 (write) */
	vdpport &= 0xFF;
	vdpport |= t->w << 8;	/* Set up width counters */
	/* Turn on the IRQ if we need it */
	if (timer_source == TIMER_TMS9918A) {
		tms9918a_ctrl = t->inton;
		tms9918a_ctrl = 0x81;
	}
	tms9918a_udgload();
	tms9918a_attributes();
	tms_mode[0].hardware = tms9918a_present;
	tms_mode[1].hardware = tms9918a_present;
}

static struct fontinfo fonti[] = {
	{ 0, 255, 128, 159, FONT_INFO_6X8 },
	{ 0, 255, 128, 159, FONT_INFO_8X8 },
};

static uint8_t igrbmsx[16] = {
	1,			/* 0000 to Black */
	4,			/* 000B to 4 dark blue */
	6,			/* 00R0 to 6 dark red */
	13,			/* 00RB to magneta */
	12,			/* 0G00 to dark green */
	7,			/* 0G0B to cyan */
	10,			/* 0GR0 to dark yellow */
	14,			/* 0GRB to grey */
	14,			/* I000 to grey */
	5,			/* I00B to light blue */
	9,			/* 10R0 to light red */
	8,			/* 10RB to magenta or medium red ? - use mr for now */
	3,			/* 1G00 to light green */
	7,			/* 1G0B to cyan - no light cyan */
	11,			/* 1GR0 to light yellow */
	15			/* 1GRB to white */
};

static uint8_t igrb_to_msx(uint8_t c)
{
	/* Machine specific colours */
	if (c & 0x10)
		return c & 0x0F;
	/* IGRB colours */
	return igrbmsx[c & 0x0F];
}

int tms_ioctl(uint8_t minor, uarg_t arg, char *ptr)
{
	uint_fast8_t is_wr = 0;
	unsigned i = 0;
	unsigned topchar = 256;
	uint8_t c;
	uint8_t map[8];

	switch (arg) {
	case GFXIOC_GETINFO:
		return uput(&tms_mode[mode[minor]], ptr, sizeof(struct display));
	case GFXIOC_MAP:
		if (vswitch)
			return -EBUSY;
		vswitch = minor;
		return uput(&tms_map, ptr, sizeof(struct videomap));
	case GFXIOC_UNMAP:
		if (vswitch == minor) {
			tms9918a_reset();
			tms9918a_reload();
			vswitch = 0;
		}
		return 0;
	case GFXIOC_GETMODE:
	case GFXIOC_SETMODE:{
			uint8_t m = ugetc(ptr);
			if (m > 1) {
				udata.u_error = EINVAL;
				return -1;
			}
			if (arg == GFXIOC_GETMODE)
				return uput(&tms_mode[m], ptr, sizeof(struct display));
			mode[minor] = m;
			if (minor == inputtty) {
				/* Input screen changed mode and size */
				tms9918a_reload();
				vt_twidth = tms_width;
				vt_tright = tms_width - 1;
			}
			return 0;
		}
	case GFXIOC_WAITVB:
		psleep(&shadowcon);
		return 0;
	case VDPIOC_SETUP:
		/* Must be locked to issue */
		if (vswitch != minor) {
			udata.u_error = EINVAL;
			return -1;
		}
		if (uget(ptr, map, 8) == -1)
			return -1;
		map[1] |= 0x80;
		if (timer_source == TIMER_TMS9918A)
			map[1] |= 0x20;
		tms9918a_config(map);
		return 0;
	case VDPIOC_READ:
		is_wr = 1;
	case VDPIOC_WRITE:
		{
			struct vdp_rw rw;
			uint16_t size;
			uint8_t *addr;
			if (vswitch != minor) {
				udata.u_error = EINVAL;
				return -1;
			}
			if (uget(ptr, &rw, sizeof(struct vdp_rw)))
				return -1;
			addr = (uint8_t *) rw.data;
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
	case VTFONTINFO:
		return uput(fonti + mode[minor], ptr, sizeof(struct fontinfo));
	case VTSETUDG:
		i = ugetc(ptr);
		ptr++;
		if (i < 128 || i >= 159) {
			udata.u_error = EINVAL;
			return -1;
		}
		topchar = i + 1;
	case VTSETFONT:
		while (i < topchar) {
			if (uget(ptr, map, 8) == -1) {
				udata.u_error = EFAULT;
				return -1;
			}
			ptr += 8;
			tms9918a_set_char(i++, map);
		}
		tms9918a_udgsave();
		tms9918a_udgload();
		return 0;
	case VTBORDER:
		c = ugetc(ptr);
		tmsborder[minor] = igrb_to_msx(c & 0x1F);
		tms9918a_attributes_m(minor);
		return 0;
	case VTINK:
		c = ugetc(ptr);
		tmsinkpaper[minor] &= 0x0F;
		tmsinkpaper[minor] |= igrb_to_msx(c & 0x1F) << 4;
		tms9918a_attributes_m(minor);
		return 0;
	case VTPAPER:
		c = ugetc(ptr);
		tmsinkpaper[minor] &= 0xF0;
		tmsinkpaper[minor] |= igrb_to_msx(c & 0x1F);
		tms9918a_attributes_m(minor);
		return 0;
	case CPUIOC_Z80SOFT81:
		if (arg == 0)
			return softzx81_off(minor);
		else
			return softzx81_on(minor);
	}
	return vtzx_ioctl(minor, arg, ptr);
}

void tms_set_mode(uint8_t minor)
{
	if (vidmode != mode[minor])
		tms9918a_reload();
	else {
		tms9918a_udgload();
		tms9918a_attributes();
	}
}

/*
 *	Console driver for the TMS9918A
 */

uint8_t tms_intr(uint8_t minor)
{
	used(minor);
	return 1;
}

void tms_setup(uint8_t minor)
{
	used(minor);
}

uint8_t tms_writeready(uint_fast8_t minor)
{
	used(minor);
	return TTY_READY_NOW;
}

void tms_setoutput(uint_fast8_t minor)
{
	vt_save(&ttysave[outputtty - 1]);
	outputtty = minor;
	curvid = VID_TMS9918A;
	vt_twidth = tms_width;
	vt_tright = tms_width - 1;
	vt_theight = 24;
	vt_tbottom = 23;
	vt_load(&ttysave[outputtty - 1]);
}

static void tms_putc(uint_fast8_t minor, uint_fast8_t c)
{
	irqflags_t irq = di();

	if (outputtty != minor)
		tms_setoutput(minor);
	irqrestore(irq);
	curvid = VID_TMS9918A;
	if (!vswitch)
		vtoutput(&c, 1);
}

struct uart tms_uart = {
	tms_intr,
	tms_writeready,
	tms_putc,
	tms_setup,
	carrier_unwired,
	_CSYS,
	"TMS9918A"
};
