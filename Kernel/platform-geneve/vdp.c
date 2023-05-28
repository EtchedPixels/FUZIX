/*
 * TEXT2 mode VT
 */

#include <kernel.h>
#include <vt.h>
#include <v99xx.h>
#include <printf.h>
#include <xtkbd.h>

#define VT_BASE 0x0000
#define VT_BASE_FONT 0x1000
#define VT_BASE_BLINK 0x800
#define VT_BUFSIZE 80
#define VT_OFFSET 0x2000

extern void *fontdata_6x8;

static uint8_t vt_buff[VT_BUFSIZE];
static uint16_t cur_blink_addr = 0;
static uint16_t vt_offset;


void vdpinit(void)
{
    uint8_t i;

    v99xx_set_mode(MODE_TEXT2);
    v99xx_set_color(15, 4);
    v99xx_copy_to_vram(VT_BASE_FONT + 32*8, (uint8_t *)&fontdata_6x8, 768);
    v99xx_set_blink_color(15, 8);
    v99xx_set_blink_period(4, 4);

    /* clean vram for all vt's */
    for (i = 0; i < MAX_VT; i++) {
        v99xx_set_vram_page(i/2);
        v99xx_memset_vram(VT_BASE + i * VT_OFFSET, ' ',  VT_HEIGHT * VT_WIDTH);
        v99xx_memset_vram(VT_BASE_BLINK + i * VT_OFFSET, 0,
                VT_HEIGHT * VT_WIDTH / 8);
    }
    vt_offset = 0;
}

void clear_lines(int8_t y, int8_t ct)
{
    uint16_t addr;

    addr = VT_BASE + vt_offset + y * VT_WIDTH;
    v99xx_set_vram_page(addr / V99xx_VRAM_PAGE_SIZE);
    if (ct)
        v99xx_memset_vram(addr, ' ',  ct * VT_WIDTH);
}

void clear_across(int8_t y, int8_t x, int16_t l)
{
    uint16_t addr;

    addr = VT_BASE + vt_offset + y * VT_WIDTH + x;
    v99xx_set_vram_page(addr / V99xx_VRAM_PAGE_SIZE);
    if (l)
        v99xx_memset_vram(addr, ' ', l);
}

void cursor_off(void)
{
    v99xx_set_vram_page(cur_blink_addr / V99xx_VRAM_PAGE_SIZE);
    v99xx_write_vram(cur_blink_addr, 0);
}

void cursor_on(int8_t y, int8_t x)
{
    uint16_t blink_addr;
    uint8_t bit;

    blink_addr = VT_BASE_BLINK + vt_offset + y * 10 + (x >> 3);
    bit = 7 - (x & 0x7);
    v99xx_set_vram_page(blink_addr / V99xx_VRAM_PAGE_SIZE);
    v99xx_write_vram(blink_addr, 1 << bit);
    cur_blink_addr = blink_addr;
}

void cursor_disable(void)
{
}

void memcpy_vram(uint16_t dst, uint16_t src, uint16_t size)
{
    uint16_t i;
    v99xx_set_vram_page(dst / V99xx_VRAM_PAGE_SIZE);
    for (i = 0; i < size; i += VT_BUFSIZE) {
	v99xx_copy_from_vram(vt_buff, src + i, VT_BUFSIZE);
	v99xx_copy_to_vram(dst + i, vt_buff, VT_BUFSIZE);
    }
}

void scroll_up(void)
{
    memcpy_vram(VT_BASE + vt_offset , VT_WIDTH + vt_offset, VT_WIDTH * VT_BOTTOM);
}

void scroll_down(void)
{
    memcpy_vram(VT_WIDTH + vt_offset , VT_BASE + vt_offset , VT_WIDTH * VT_BOTTOM);
}

void plot_char(int8_t y, int8_t x, uint16_t c)
{
    uint16_t addr;
    addr = VT_BASE + vt_offset + y * VT_WIDTH + x;
    v99xx_set_vram_page(addr / V99xx_VRAM_PAGE_SIZE);
    v99xx_write_vram(addr, c);
}

void vtattr_notify(void)
{
}

void set_active_vt(uint8_t vt)
{
    vt_offset = vt * VT_OFFSET;
}

void set_visible_vt(uint8_t vt)
{
    uint8_t val;
    uint16_t offset = vt * VT_OFFSET;

    val = (uint8_t)(((VT_BASE + offset & 0xF800) >> 10) | 0x03);
    v99xx_write_reg(V99xx_REG_PTRN_LAYOUT_BASE, val);

    val = (uint8_t)(((VT_BASE_BLINK + offset & 0x3F00) >> 6) | 0x07);
    v99xx_write_reg(V99xx_REG_COLOR_BASE_H, val);

    val = (uint8_t)(((VT_BASE_BLINK + offset & 0xC000) >> 14) & 0x07);
    v99xx_write_reg(V99xx_REG_COLOR_BASE_L, val);
}

unsigned int inputtty;

void xtkbd_conswitch(uint_fast8_t console)
{
    inputtty = console;
    set_visible_vt(console);
}
