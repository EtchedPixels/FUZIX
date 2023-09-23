#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <tty.h>
#include <vt.h>
#include <graphics.h>
#include <vdp1.h>
#include <devtty.h>
#include "adam.h"

static char tbuf1[TTYSIZ];

int8_t vt_twidth = 40;
int8_t vt_tright = 39;

struct vt_repeat keyrepeat;
uint8_t vtattr_cap;

uint8_t inputtty = 1;
uint8_t outputtty = 1;		/* Fixed but needed by the VDP layer */
uint8_t vidmode = 0;		/* Will be needed when we add the mode setting */
uint8_t vswitch;

static uint8_t tmsinkpaper[2] = {0, 0xF4 };
static uint8_t tmsborder[2] = { 0, 0x04 };

static uint8_t sleeping;

/* For now just do TMS9918A console basics.
   TODO: console flipping and serial options */
struct s_queue ttyinq[NUM_DEV_TTY + 1] = {	/* ttyinq[0] is never used */
	{NULL, NULL, NULL, 0, 0, 0},
	{tbuf1, tbuf1, tbuf1, TTYSIZ, 0, TTYSIZ / 2},
};

tcflag_t termios_mask[NUM_DEV_TTY + 1] = {
	0,
	_CSYS
};

void tty_setup(uint_fast8_t minor, uint_fast8_t flags)
{
}

int tty_carrier(uint_fast8_t minor)
{
	return 1;
}

/* Poll the console */
void tty_pollirq(void)
{
	/* Go via firmware */
	unsigned c = keycheck();
	/* Error - just hope it goes away */
	if (c == 0xFF)
		return;
	if (c) {
		tty_inproc(1, c);
		keypoll();
	}
}

void tty_putc(uint_fast8_t minor, uint_fast8_t c)
{
	vtoutput(&c, 1);
}

void tty_sleeping(uint_fast8_t minor)
{
	sleeping |= (1 << minor);
}

ttyready_t tty_writeready(uint_fast8_t minor)
{
	return TTY_READY_NOW;
}

void tty_data_consumed(uint_fast8_t minor)
{
	used(minor);
}

/* kernel writes to system console -- never sleep! */
void kputchar(uint_fast8_t c)
{
	while(tty_writeready(TTYDEV - 512) != TTY_READY_NOW);
	if (c == '\n')
		tty_putc(TTYDEV - 512, '\r');
	while(tty_writeready(TTYDEV - 512) != TTY_READY_NOW);
	tty_putc(TTYDEV - 512, c);
}

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
	0x98,		/* FIXME: update at boot ? */
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

  if (minor == 1) {
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
#if 0
	/* For now - needs a custom rop/wop handler */		
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
		size = rw.lines * rw.cols;
		addr = (uint8_t *)rw.data;
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
#endif	
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
			if (uget(ptr, map, 8) == -1) {
				udata.u_error = EFAULT;
				return -1;
			}
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
