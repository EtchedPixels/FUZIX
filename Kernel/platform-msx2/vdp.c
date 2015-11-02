/*
 * TEXT2 mode VT
 */

#include <kernel.h>
#include <vt.h>
#include <v99xx.h>

/* TODO: extend to support multiple VT (or scrollable history)
 * possible to keep up to 31 pages in VRAM */
#define VT_BASE 0x0000
#define VT_BASE_FONT 0x1000
#define VT_BASE_BLINK 0x800
#define VT_BUFSIZE 80

extern void *fontdata_6x8;

static uint8_t vt_buff[VT_BUFSIZE];
static uint16_t cur_blink_addr = 0;

void vdpinit()
{
    v99xx_set_mode(MODE_TEXT2);
    v99xx_set_color(15, 4);
    v99xx_copy_to_vram(VT_BASE_FONT + 32*8, (uint8_t *)&fontdata_6x8, 768);
    v99xx_set_blink_color(15, 8);
    v99xx_set_blink_period(4, 4);
}

void clear_lines(int8_t y, int8_t ct)
{
    uint16_t addr;

    addr = VT_BASE + y * VT_WIDTH;
    v99xx_memset_vram(addr, ' ',  ct * VT_WIDTH);
}

void clear_across(int8_t y, int8_t x, int16_t l)
{
    uint16_t addr;

    addr = VT_BASE + y * VT_WIDTH + x;
    v99xx_memset_vram(addr, ' ', l);
}

void cursor_off(void)
{
    v99xx_write_vram(cur_blink_addr, 0);
}

void cursor_on(int8_t y, int8_t x)
{
    uint16_t blink_addr;
    uint8_t bit;

    blink_addr = VT_BASE_BLINK + y * 10 + (x >> 3);
    bit = 7 - (x & 0x7);

    v99xx_write_vram(blink_addr, 1 << bit);
    cur_blink_addr = blink_addr;
}

void memcpy_vram(uint16_t dst, uint16_t src, uint16_t size)
{
    uint16_t i;

    for (i = 0; i < size; i += VT_BUFSIZE) {
	v99xx_copy_from_vram(vt_buff, src + i, VT_BUFSIZE);
	v99xx_copy_to_vram(dst + i, vt_buff, VT_BUFSIZE);
    }
}

void scroll_up(void)
{
    memcpy_vram(VT_BASE, VT_WIDTH, VT_WIDTH * VT_BOTTOM);
}

void scroll_down(void)
{
    memcpy_vram(VT_WIDTH, VT_BASE, VT_WIDTH * VT_BOTTOM);
}

void plot_char(int8_t y, int8_t x, uint16_t c)
{
    uint16_t addr;

    addr = VT_BASE + y * VT_WIDTH + x;
    v99xx_write_vram(addr, c);
}

void vtattr_notify(void)
{
}
