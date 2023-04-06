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
 */

#include <kernel.h>
#include <tty.h>
#include <vt.h>
#include <devtty.h>
#include <rcbus.h>
#include "printf.h"
#include "multivt.h"

#define CMD_MODE	0x00
#define CMD_HSCROLL	0x03
#define CMD_VSCROLL	0x04
#define CMD_SPRITE	0x0C
#define CMD_TILEMAP	0x0D
#define CMD_TILEBITS	0x0E

extern uint8_t fontdata_6x8[];

#ifdef CONFIG_RC2014_EXTREME
__sfr __banked __at 0x40B8	vid_cmd;
__sfr __banked __at 0x41B8	vid_data;
__sfr __banked __at 0x68B8	vid_pio;	/* TODO pick better port */

#define VID_PIO_WAIT		0x40

/* There is no HALT on the bus extender 40 pins so use a GPIO input */
static void nap(void)
{
    while(!(vid_pio & VID_PIO_WAIT));
}

#else
#define nap()		do {} while(0)
__sfr __at 0x40		vid_cmd;
__sfr __at 0x41		vid_data;
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
	nap();
	vid_data = off;
	nap();
	vid_data = off >> 8;
	nap();
}

void ma_cursor_off(void)
{
}

void ma_cursor_disable(void)
{
        vid_cmd = CMD_SPRITE;
	nap();
        vid_data = 0x00;	/* Sprite 0 is the cursor */
	nap();
        vid_data = 0x00;
	nap();
        vid_data = 0x00;
	nap();
        vid_data = ' ';
	nap();
        vid_data = 0x00;
	nap();
}

void ma_cursor_on(int8_t y, int8_t x)
{
        vid_cmd = CMD_SPRITE;
	nap();
        vid_data = x;
	nap();
        vid_data = y;
	nap();
        vid_data = 0;		/* For testing */
	nap();
        if (x & 0x80) 
            vid_data = 1; 
        else
            vid_data = 0;
	nap();
}

void ma_plot_char(int8_t y, int8_t x, uint16_t c)
{
        tilemap_set(y, x);
        vid_data = c;
	nap();
}

void ma_clear_lines(int8_t y, int8_t ct)
{
        uint16_t n = (uint8_t)ct * 40;
        tilemap_set(y, 0);
        while(n--)
            vid_data = ' ';
}

void ma_clear_across(int8_t y, int8_t x, int16_t l)
{
        tilemap_set(y, x);
        while(l--) {
            vid_data = ' ';
            nap();
        }
}

void ma_vtattr_notify(void)
{
}

/* No read back so we must hardware scroll - which is faster anyway */
void ma_scroll_up(void)
{
        scroll_pos ++;
        if (scroll_pos == 30)
            scroll_pos = 0;
        vid_cmd = CMD_VSCROLL;
        nap();
        vid_data = scroll_pos << 3;
        nap();
        vid_data = 0;
}

void ma_scroll_down(void)
{
        if (scroll_pos == 0)
                scroll_pos = 29;
        else
                scroll_pos--;
        vid_cmd = CMD_VSCROLL;
        nap();
        vid_data = scroll_pos << 3;
        nap();
        vid_data = 0;
        nap();
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
 
/* TODO */          
void macca_set_shown(void)
{
        vid_cmd = CMD_HSCROLL;
        nap();
        if (outputtty == 2) {
            vid_data = 320 & 0xFF;
            nap();
            vid_data = 320 >> 8;
            nap();
        } else {
            vid_data = 0;
            nap();
            vid_data = 0;
            nap();
        }
}

/* Should be in discard... */

/* 320 x 240 40 x 30 */
void macca_init(void)
{
        uint8_t *p;
        unsigned n = 0;
        unsigned i;

        vid_cmd = CMD_MODE;
        nap();
        vid_data = 0x80;	/* 320x240 double tilewidth */
        nap();
        vid_data = 0x00;
        nap();
        vid_data = 0x00;
        nap();

        /* Sanity check */
        vid_cmd = CMD_TILEBITS;
        nap();
        vid_data = 0x00;
        nap();
        vid_data = 0x00;
        nap();
        for (i = 0; i < 64; i++) {
            vid_data = 0xAA;
            nap();
        }

        /* Load the font */
        p = fontdata_6x8;
        vid_cmd = CMD_TILEBITS;
        nap();
        /* We only load 32 to 127 */
        vid_data = 0x00;
        nap();
        vid_data = 0x08;	/* 2K in */
        nap();

        while(n++ < 768) {
                uint8_t i;
                uint8_t b = *p++;
                /* Expand each pixel row into a tile row of 8 bytes */
                for (i = 0; i < 8; i++) {
                        if (b & 0x80)
                                vid_data = 0xFF;
                        else
                                vid_data = 0x00;
                        nap();
                        b <<= 1;
                }
        }
}

/* TODO: UDG and font loading ioctl support */
/* TODO: sprites ioctls */
/* TODO: mode setting ioctls */

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

static void macca_putc(uint_fast8_t minor, uint_fast8_t c)
{
	irqflags_t irq = di();

/* TODO	if (outputtty != minor)
		ma_setoutput(minor); */
	irqrestore(irq);
	curvid = VID_MACCA;
	if (!vswitch)
		vtoutput(&c, 1);
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
	macca_intr,			/* TODO */
	macca_writeready,
	macca_putc,
	macca_setup,
	carrier_unwired,
	_CSYS,
	"PropGfx"
};
