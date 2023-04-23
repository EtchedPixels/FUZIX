/*
 *	EF9345 Video.
 *	TODO:
 *	Maybe multi-con (but it's horrible)
 *	Attributes. The hardware supports inverse, flash
 *
 *	Colours
 *	- why the need for activity to sort out 40 col on a flip ?
 */

#include <kernel.h>
#include <kdata.h>
#include <tty.h>
#include <vt.h>
#include <devtty.h>
#include <rcbus.h>
#include <graphics.h>
#include "printf.h"
#include "multivt.h"

#ifdef CONFIG_RC2014_EXTREME
__sfr __banked __at 0x44B8 reg;
__sfr __banked __at 0x46B8 data;
#else
__sfr __at 0x44 reg;
__sfr __at 0x46 data;
#endif

static uint8_t scrollpos;
static uint8_t ef_mode;
static uint8_t efinkpaper[5] = { 0, 0x07, 0x07, 0x07, 0x07 };

static uint8_t efborder[5];
static uint8_t cursordis;
static uint8_t matreg;
static uint8_t attr = 0x70;

static const uint8_t modetab[2][5] = {
	{
		0x10,		/* PAL, combined sync */
		0x48,		/* Fixed cursor and enabled I high during margin
				   margin is black, no zoom */
		0x7E,		/* service row off, upper/lower bulk on, conceal on
				   i high during displayed area flash on, 40 col long code */
		 0x13,		/* alpha uds slices in block 3, semi in blocks 2/3
				   quadrichrome in block 0 */
		 0x28,		/* origin row 8, display block 0 ; 28 08 ? */
	}, {
		0xD0,		/* 80 char PAL combined sync */
		0x48,		/* Fixed cursor and enabled I high during
				   margin. Margin is black, no zoom */
		0x7E,		/* Turn it all on except status bar */
		0x8F,		/* white on black */
		0x08,		/* origin row 8 block 0 for display */
       }
};

/* FIXME: inline define */
static void waitrdy(void)
{
	reg = 0x20;
	while (data & 0x80);
}

uint8_t ef9345_probe(void)
{
	reg = 0x00;

	data = 0xAA;
	if (data == 0xAA)
		return 0;
	reg = 0x23;
	data = 0xAA;
	if (data != 0xAA)
		return 0;
	data = 0x55;
	if (data != 0x55)
		return 0;
	return 1;
}

static void ef_load_indirect(uint8_t r, uint8_t v)
{
	reg = 0x21;		/* Set R1 to the value */
	data = v;
	reg = 0x28;		/* Execute a load indirect command */
	data = r | 0x80;
	waitrdy();
}

static void ef_load_mode(uint8_t m)
{
	const uint8_t *p = &modetab[m][0];

	reg = 0x20;
	data = 0x91;		/* Force a nop */
	waitrdy();

	ef_load_indirect(1, *p++);
	matreg = *p++;
	ef_load_indirect(2, matreg | efborder[inputtty]);
	ef_load_indirect(3, *p++);
	ef_load_indirect(4, *p++);
	ef_load_indirect(7, *p++);
	ef_mode = m;
	/* if we do multi console we will need top be smarter about scrollpos */
	scrollpos = 0;
	if (m == 0) {
		vt_twidth = 40;
		vt_tright = 39;
	} else {
		vt_twidth = 80;
		vt_tright = 79;
	}
}

void ef9345_init(void)
{
	ef_load_mode(1);
	/* Clear display */
	ef_clear_lines(0, 24);
}

static void ef_set_pos(int8_t y, uint8_t x)
{
	uint8_t d;

	reg = 0x27;
	if (ef_mode == 1) {
		d = x >> 1;
		if (x & 1)
			d |= 0x80;
		data = d;
	} else
		data = x;
	reg = 0x26;
	y += scrollpos;
	if (y >= 24)
		y -= 24;
	data = y + 8;
	/* Set up KRL80 */
	waitrdy();
	if (ef_mode == 1) {
		reg = 0x23;
		data = 0x00;	/* Attributes clear for now */
		reg = 0x20;
		data = 0x51;	/* KRL with increment */
	} else {
		reg = 0x22;
		data = 0x00;
		reg = 0x23;
		data = attr;
		reg = 0x20;
		data = 0x01;
	}
}

/* Another one that needs teaching banking */
static void ef_set_scroll(void)
{
	uint8_t v = scrollpos + 8;
	ef_load_indirect(7, v);
}

void ef_plot_char(int8_t y, int8_t x, uint16_t ch)
{
	ef_set_pos(y, x);
	waitrdy();
	reg = 0x29;
	data = ch;
}

void ef_scroll_down(void)
{
	if (scrollpos == 0)
		scrollpos = 23;
	else
		scrollpos--;
	ef_set_scroll();
}

void ef_scroll_up(void)
{
	scrollpos++;
	if (scrollpos == 24)
		scrollpos = 0;
	ef_set_scroll();
}

void ef_clear_across(int8_t y, int8_t x, int16_t l)
{
	ef_set_pos(y, x);
	while (l--) {
		waitrdy();
		reg = 0x29;
		data = 0x20;
	}
}

void ef_clear_lines(int8_t y, int8_t ct)
{
	while (ct--)
		ef_clear_across(y++, 0, vt_twidth);
}

void ef_cursor_on(int8_t y, int8_t x)
{
	if (cursordis) {
		cursordis = 0;
		matreg |= 0x40;
		ef_load_indirect(2, matreg);
	}
}

void ef_cursor_disable(void)
{
	cursordis = 1;
	matreg &= ~0x40;
	ef_load_indirect(2, matreg);
}

void ef_cursor_off(void)
{
}

void ef9345_set_output(void)
{
}

void ef_set_console(void)
{
}

void ef9345_colour(uint16_t cpair)
{
	if (ef_mode == 0)
		return;
	ef_load_indirect(4, cpair);
}

static uint8_t make_ibgr(uint8_t c)
{
	uint8_t r = 0;
	if (c & 0x20)
		return c & 15;
	if (c & 1)		/* Blue */
		r |= 4;
	if (c & 2)		/* Red */
		r |= 1;
	if (c & 4)		/* Green */
		r |= 2;
	return r;
}

/* IGRB colours but hardware is IBGR */
void ef_vtattr_notify(void)
{
	attr = (make_ibgr(vtink) << 4) | make_ibgr(vtpaper);
}

static struct videomap ef_map = {
	0,
	0x44,
	0, 0,
	0, 0,
	2,
	MAP_PIO
};

static struct display ef_modes[2] = {
	{
	 0,
	 40, 24,
	 40, 24,
	 255, 255,
	 FMT_TEXT,
	 HW_EF9345,
	 GFX_MULTIMODE | GFX_MAPPABLE | GFX_TEXT,	/* TODO: vblank */
	 16,
	 0,
	 40, 24 },
	{
	 1,
	 80, 24,
	 80, 24,
	 255, 255,
	 FMT_TEXT,
	 HW_EF9345,
	 GFX_MULTIMODE | GFX_MAPPABLE | GFX_TEXT,	/* TODO: vblank */
	 16,
	 0,
	 80, 24 }
};

int ef_ioctl(uint8_t minor, uarg_t arg, char *ptr)
{
	uint8_t c;
	switch (arg) {
	case GFXIOC_GETINFO:
		return uput(ef_modes + ef_mode, ptr, sizeof(struct display));
	case GFXIOC_MAP:
		return uput(&ef_map, ptr, sizeof(struct videomap));
	case GFXIOC_UNMAP:
		return 0;
	case GFXIOC_GETMODE:
	case GFXIOC_SETMODE:{
		uint8_t m = ugetc(ptr);
		if (m > 1) {
			udata.u_error = EINVAL;
			return -1;
		}
		if (arg == GFXIOC_GETMODE)
			return uput(ef_modes + m, ptr, sizeof(struct display));
		ef_mode = m;
		ef_load_mode(m);
		ef9345_colour(efinkpaper[minor]);
		ef_clear_lines(0, 24);
		return 0;
	}
	case VTINK:
		if (ef_mode == 0)
			return -1;
		c = make_ibgr(ugetc(ptr));
		efinkpaper[minor] &= 0xF8;
		efinkpaper[minor] |= c & 0x07;
		if (minor == inputtty)
			ef9345_colour(efinkpaper[minor]);
		return 0;
#if 0
		/* Not clear how/if we can use this */
	case VTPAPER:
		/* Mode 0 has per character colour */
		if (ef_mode == 0)
			return -1;
		c = make_ibgr(ugetc(ptr));
		efinkpaper[minor] &= 0x8F;
		efinkpaper[minor] |= (c & 0x07) << 4;
		if (minor == inputtty)
			ef9345_colour(efinkpaper[minor]);
		return 0;
#endif
	case VTBORDER:
		c = make_ibgr(ugetc(ptr));
		efborder[minor] = c & 0x07;
		if (minor == inputtty) {
			matreg &= 0xF8;
			matreg |= efborder[minor];
			ef_load_indirect(2, matreg);
		}
		return 0;
	}
	return vtzx_ioctl(minor, arg, ptr);
}

static uint8_t ef_intr(uint8_t minor)
{
	return 1;
}

static ttyready_t ef_writeready(uint8_t minor)
{
	return TTY_READY_NOW;
}

static void ef_setup(uint8_t minor)
{
	used(minor);
}

void ef_setoutput(uint_fast8_t minor)
{
	vt_save(&ttysave[outputtty - 1]);
	outputtty = minor;
	curvid = VID_EF9345;
	if (ef_mode == 1) {
		vt_twidth = 80;
		vt_tright = 79;
	} else {
		vt_twidth = 40;
		vt_tright = 39;
	}
	vt_theight = 24;
	vt_tbottom = 23;
	ef9345_set_output();
	vt_load(&ttysave[outputtty - 1]);
}

static void ef_putc(uint_fast8_t minor, uint_fast8_t c)
{
	irqflags_t irq = di();

	if (outputtty != minor)
		ef_setoutput(minor);
	irqrestore(irq);
	curvid = VID_EF9345;
	if (!vswitch)
		vtoutput(&c, 1);
}

struct uart ef_uart = {
	ef_intr,		/* TODO */
	ef_writeready,
	ef_putc,
	ef_setup,
	carrier_unwired,
	_CSYS,
	"EF9345"
};
