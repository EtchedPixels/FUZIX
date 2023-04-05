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
#include "multivt.h"

#define CMD_MODE	0x00
#define CMD_HSCROLL	0x03
#define CMD_VSCROLL	0x04
#define CMD_SPRITE	0x0C
#define CMD_TILEMAP	0x0D
#define CMD_TILEBITS	0x0E

extern uint8_t fontdata_6x8[];

__sfr __at 0x40		vid_cmd;
__sfr __at 0x41		vid_data;

static uint8_t scroll_pos;
static uint8_t con_xbias;

static void tilemap_set(signed char y1, unsigned char x1)
{
	uint16_t off;
	
	y1 -= scroll_pos;
	if (y1 < 0)
	    y1 += 30;
	
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
        vid_cmd = CMD_SPRITE;
        vid_data = 0x00;	/* Sprite 0 is the cursor */
        vid_data = 0x00;
        vid_data = 0x00;
        vid_data = ' ';
        vid_data = 0x00;
}

void ma_cursor_on(int8_t y, int8_t x)
{
        vid_cmd = CMD_SPRITE;
        vid_data = x;
        vid_data = y;
        vid_data = '_';		/* For testing */
        if (x & 0x80) 
            vid_data = 1; 
        else
            vid_data = 0;
}

void ma_plot_char(int8_t y, int8_t x, uint16_t c)
{
        tilemap_set(y, x);
        vid_data = c;
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
        while(l--)
            vid_data = ' ';
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
        if (outputtty)
                con_xbias = 40;
        else
                con_xbias = 0;
}
 
/* TODO */          
void macca_set_shown(void)
{
        vid_cmd = CMD_HSCROLL;
        if (outputtty) {
            vid_data = 320 & 0xFF;
            vid_data = 320 >> 8;
        } else {
            vid_data = 0;
            vid_data = 0;
        }
}

/* Should be in discard... */

/* 320 x 240 40 x 30 */
void macca_init(void)
{
        uint8_t *p;
        unsigned n = 0;

        vid_cmd = CMD_MODE;
        vid_data = 0x80;	/* 320x240 double tilewidth */
        vid_data = 0x00;
        vid_data = 0x00;

        /* Load the font */
        p = fontdata_6x8;
        vid_cmd = CMD_TILEBITS;
        /* We only load 32 to 127 */
        vid_data = 0x00;
        vid_data = 0x04;

        while(n++ < 768) {
                uint8_t i;
                uint8_t b = *p++;
                /* Expand each pixel row into a tile row of 8 bytes */
                for (i = 0; i < 8; i++) {
                        if (b & 0x80)
                                vid_data = 0xFF;
                        else
                                vid_data = 0x00;
                        b <<= 1;
                }
        }
}

/* TODO: UDG and font loading ioctl support */
/* TODO: sprites ioctls */
/* TODO: mode setting ioctls */
