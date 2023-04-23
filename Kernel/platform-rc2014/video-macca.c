/*
 *	Maccasoft propellor graphics card
 *
 *	In theory this is a tile and sprite based video interface with
 *	hardware scrolling. In practice of course if you make the tiles
 * 	a font that turns it into a very nice fast 40 column vga terminal.
 *
 *	We don't do clever stuff with the font at this point, but because
 *	the font is the full 8bit colour range you can actually upload
 *	anti-aliased or phosphor fuzzed fonts....
 *
 *	The big limitation is around colour. The tiles have their own colour
 *	data so you can't do multiple text colours.
 *
 *	The documentation is a bit unclear on a few things
 *	- The offsets for the video are set when you change mode automatially
 *	- The tile expand does not double the on screen resolution just makes
 *	  scrolling maps easier. We can use this for dual console
 *	- The colours are in the upper bits of the tile. Bit 0 is 1 for
 *	  transparency on sprites and overlays
 *	- The wait pin can be ignored if you wait long enough on a mode
 *	  change or a bitmap mode clear screen (we do)
 */

#include <kernel.h>
#include <kdata.h>
#include <tty.h>
#include <vt.h>
#include <devtty.h>
#include <rcbus.h>
#include <graphics.h>

#include "multivt.h"

#define CMD_MODE	0x00
#define CMD_HSCROLL	0x03
#define CMD_VSCROLL	0x04
#define CMD_SPRITE	0x0C
#define CMD_TILEMAP	0x0D
#define CMD_TILEBITS	0x0E

extern uint8_t fontdata_6x8[];

#ifdef CONFIG_RC2014_EXTREME
__sfr __banked __at 0x40B8 vid_cmd;
__sfr __banked __at 0x41B8 vid_data;
__sfr __at 0x68 vid_pio;

#else

__sfr __at 0x40 vid_cmd;
__sfr __at 0x41 vid_data;

#endif

static uint8_t scroll_pos;
static uint8_t con_xbias;

static void tilemap_set(unsigned char y1, unsigned char x1)
{
	uint16_t off;

	/* Adjust for hardware scroll */
	y1 += scroll_pos;
	if (y1 >= 30)
		y1 -= 30;

	off = 80 * y1 + x1 + con_xbias;
	vid_cmd = CMD_TILEMAP;
	vid_data = off;
	vid_data = off >> 8;
}

void ma_cursor_off(void)
{
}

void ma_cursor_disable(void)
{
	if (inputtty == outputtty) {
		vid_cmd = CMD_SPRITE;
		vid_data = 0x00;	/* Sprite 0 is the cursor */
		vid_data = 0x00;
		vid_data = 0x00;
		vid_data = 0x00;
		vid_data = 0x00;
	}
}

void ma_cursor_on(int8_t y, int8_t x)
{
	if (inputtty == outputtty) {
		vid_cmd = CMD_SPRITE;
		vid_data = 0;
		vid_data = x << 3;
		vid_data = y << 3;
		vid_data = 1;	/* For testing */
		if (x > 31)
			vid_data = 1;
		else
			vid_data = 0;
	}
}

void ma_plot_char(int8_t y, int8_t x, uint16_t c)
{
	tilemap_set(y, x);
	vid_data = c;
}

void ma_clear_lines(int8_t y, int8_t ct)
{
	while (ct--)
		ma_clear_across(y++, 0, 40);
}

void ma_clear_across(int8_t y, int8_t x, int16_t l)
{
	tilemap_set(y, x);
	while (l--)
		vid_data = ' ';
}

void ma_vtattr_notify(void)
{
}

/* No read back so we must hardware scroll - which is faster anyway */
void ma_scroll_up(void)
{
	scroll_pos++;
	if (scroll_pos == 30)
		scroll_pos = 0;
	vid_cmd = CMD_VSCROLL;
	vid_data = scroll_pos << 3;
	vid_data = 0;
}

void ma_scroll_down(void)
{
	if (scroll_pos == 0)
		scroll_pos = 29;
	else
		scroll_pos--;
	vid_cmd = CMD_VSCROLL;
	vid_data = scroll_pos << 3;
	vid_data = 0;
}

/* Console 0 is the left of the expanded tile map
   Console 1 is the right */
void macca_set_output(void)
{
	if (outputtty == 2)
		con_xbias = 40;
	else
		con_xbias = 0;
}

void ma_set_console(void)
{
	vid_cmd = CMD_HSCROLL;
	if (outputtty == 2) {
		vid_data = 320 & 0xFF;
		vid_data = 320 >> 8;
	} else {
		vid_data = 0;
		vid_data = 0;
	}
}

/* Until we have a proper "set tile" API */
static void macca_set_char(uint8_t ch, uint8_t *p)
{
	irqflags_t i = di();
	uint8_t n = 0;

	vid_cmd = CMD_TILEBITS;
	vid_data = ch << 3;	/* Low bits x 8 */
	vid_data = ch >> 5;	/* High bits */

	while (n++ < 8) {
		uint8_t i;
		uint8_t b = *p++;
		/* Expand each pixel row into a tile row of 8 bytes */
		for (i = 0; i < 8; i++) {
			if (b & 0x80)
				vid_data = 0xFC;
			else
				vid_data = 0x08;
			b <<= 1;
		}
	}
	/* We are good */
	irqrestore(i);
}

/* 320 x 240 40 x 30 */
uint8_t macca_init(void)
{
	uint8_t *p;
	unsigned n = 0;
	unsigned i;

	vid_cmd = CMD_MODE;
	vid_data = 0x80;	/* 320x240 double tilewidth */
	vid_data = 0x00;
	vid_data = 0x00;

	/* Mode change is really slow and the device won't
	   talk to us until it is back */
	for (i = 0; i < 50000; i++);
	for (i = 0; i < 50000; i++);

	vid_cmd = CMD_TILEBITS;
	vid_data = 0x00;
	vid_data = 0x00;
	/* Set entry 0 and most of entry 1 to invisible */
	for (i = 0; i < 112; i++) {
		vid_data = 0x01;
	}
	/* Set the bottom of entry 1 to a cursor */
	while(i++ < 128) {
		vid_data = 0xFC;
	}

	/* Load the font */
	p = fontdata_6x8;
	vid_cmd = CMD_TILEBITS;
	/* We only load 32 to 127 */
	vid_data = 0x00;
	vid_data = 0x08;	/* 2K in */

	while (n++ < 768) {
		uint8_t i;
		uint8_t b = *p++;
		/* Expand each pixel row into a tile row of 8 bytes */
		for (i = 0; i < 8; i++) {
			if (b & 0x80)
				vid_data = 0xFC;
			else
				vid_data = 0x08;
			b <<= 1;
		}
	}
	ma_clear_lines(0, 30);
	return 1;
}


void ma_setoutput(uint_fast8_t minor)
{
	vt_save(&ttysave[outputtty - 1]);
	outputtty = minor;
	curvid = VID_MACCA;
	vt_twidth = 40;
	vt_tright = 39;
	vt_theight = 30;
	vt_tbottom = 29;
	macca_set_output();
	vt_load(&ttysave[outputtty - 1]);
}

static struct videomap ma_map = {
	0,
#ifdef CONFIG_RC2014_EXTREME
	0x40B8,
#else
	0x40,
#endif
	0, 0,
	0, 0,
	2,
	MAP_PIO
};

static struct display ma_mode[1] = {
	{
	 0,
	 40, 30,
	 40, 30,
	 255, 255,
	 FMT_TEXT,
	 HW_PROPGFX,
	 GFX_MAPPABLE | GFX_TEXT,
	 0,
	 0,
	 40, 30 }
};

/* TODO: sprites ioctls */

static struct fontinfo fonti = {
	0, 255, 128, 255, FONT_INFO_8X8
};

int ma_ioctl(uint8_t minor, uarg_t arg, char *ptr)
{
	unsigned topchar = 256;
	unsigned i = 0;
	uint8_t map[8];

	switch (arg) {
	case GFXIOC_GETINFO:
		return uput(&ma_mode, ptr, sizeof(struct display));
	case GFXIOC_MAP:
		return uput(&ma_map, ptr, sizeof(struct videomap));
	case GFXIOC_UNMAP:
		return 0;
	case VTFONTINFO:
		return uput(&fonti, ptr, sizeof(struct fontinfo));
	/* Our tiles are complicated because of the colour rules so until
	   we have some kind of sensible API for such things just report
	   the normal bitmap stuff and expand */
	case VTSETUDG:
		i = 128;
	case VTSETFONT:
		while (i < topchar) {
			if (uget(ptr, map, 8) == -1) {
				udata.u_error = EFAULT;
				return -1;
			}
			ptr += 8;
			macca_set_char(i++, map);
		}
		return 0;
	}
	return vtzx_ioctl(minor, arg, ptr);
}

static void macca_putc(uint_fast8_t minor, uint_fast8_t c)
{
	irqflags_t irq = di();

	if (outputtty != minor)
		ma_setoutput(minor);

	/* We have a single command stream so we need to
	   send each block uninterrupted. As we are so fast
	   anyway just keep interrupts off during the I/O */
	curvid = VID_MACCA;
	if (!vswitch)
		vtoutput(&c, 1);
	irqrestore(irq);
}

static void macca_setup(uint8_t minor)
{
	used(minor);
}

static uint8_t macca_intr(uint8_t minor)
{
	return 1;
}

static ttyready_t macca_writeready(uint8_t minor)
{
	return TTY_READY_NOW;
}

struct uart macca_uart = {
	macca_intr,		/* TODO */
	macca_writeready,
	macca_putc,
	macca_setup,
	carrier_unwired,
	_CSYS,
	"PropGfx"
};
