/* Low level VDP driver */

/* FIXME: look hard at using uint_fast8_t around here */

/*
 *	Delays need adding - only works on an emulator!
 */
#include <v99xx.h>
#include <kernel.h>

#define vdpm	((volatile uint8_t *)0xF100)
#define vdpr	((volatile uint8_t *)0xF102)

static volatile uint16_t dummy;

uint8_t vtattr_cap;

void v99xx_write_reg(uint8_t reg, uint8_t val)
{
    *vdpr = val;
    *vdpr = reg | 0x80;
}

void v99xx_set_vram_page(uint8_t page)
{
    v99xx_write_reg(V99xx_SET_VRAM_PAGE, page & 0x07);
}

/*
 * write byte in current ram page
 * (only 14 bits of address are used)
 */
void v99xx_write_vram(uint16_t addr, uint8_t val)
{
    *vdpr = addr;
    *vdpr = (addr >> 8) | 0x40;
    *vdpm = val;
}

/*
 * read byte from current ram page
 * (only 14 bits of address are used)
 */
uint8_t v99xx_read_vram(uint16_t addr)
{
    *vdpr = addr;
    *vdpr = (addr >> 8);
    dummy++;
    /* FIXME: check delay */
    return *vdpm;
}

/*
 * memset to vram current page (up to 16Kb)
 */
void v99xx_memset_vram(uint16_t addr, uint8_t value, uint16_t size)
{
    *vdpr = addr;
    *vdpr = (addr >> 8) | 0x40;
    while(size--) {
        *vdpm = value;
        dummy++;
    }
}

/*
 * Copy from vram current page (up to 16Kb)
 */
void v99xx_copy_from_vram(uint8_t *dst, uint16_t vaddr, uint16_t size)
{
    *vdpr = vaddr;
    *vdpr = vaddr >> 8;
    while(size--) {
        dummy++;
        *dst = *vdpm;
    }
}

/*
 * Copies to vram current page (up to 16Kb)
 */
void v99xx_copy_to_vram(uint16_t vaddr, uint8_t *src, uint16_t size)
{
    *vdpr = vaddr;
    *vdpr = (vaddr >> 8) | 0x40;
    while(size--) {
        *vdpm = *src++;
        dummy++;
    }
}

void v99xx_set_mode(unsigned char mode)
{
    switch (mode)
    {
	case MODE_TEXT2:
	    /* default TEXT2 configuration same as in bios */
	    v99xx_write_reg(V99xx_REG_MODE0,0x04);
	    v99xx_write_reg(V99xx_REG_MODE1,0x70);
	    v99xx_write_reg(V99xx_REG_PTRN_LAYOUT_BASE,0x03);
	    v99xx_write_reg(V99xx_REG_COLOR_BASE_H, 0x27);
	    v99xx_write_reg(V99xx_REG_COLOR_BASE_L, 0);
	    v99xx_write_reg(V99xx_REG_PATRN_GEN_BASE, 2);
	    break;
    }
}

void v99xx_set_color(uint8_t fg, uint8_t bg)
{
    v99xx_write_reg(V99xx_REG_COLOR1, (fg << 4) | (bg & 0xf));
}

void v99xx_set_blink_color(uint8_t fg, uint8_t bg)
{
    v99xx_write_reg(V99xx_REG_COLOR2, (fg << 4) | (bg & 0xf));
}

void v99xx_set_blink_period(uint8_t fg, uint8_t bg)
{
    v99xx_write_reg(V99xx_REG_BLINK_PERIOD, (fg << 4) | (bg & 0xf));
}
