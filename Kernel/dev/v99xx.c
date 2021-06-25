/*
 * v99xx video display processor driver (currently only used for the MSX2 port)
 *
 * (tms9918a, v9938 and v9958)
 *
 * [@MSX speed: 3.58MHz]
 * Timing rules:	80col Text	40col Text	 Graphic 1
 *	TMS9918A:		12		12		29
 *	VDP9938:		20		20		15
 *	VDP9958:		20		20		15
 *
 *	Our loops run at 25 and 26 clocks.
 *
 *	PLATFORM_VDP_DELAY should be defined to be the appropriate instructions
 *	for the timing of the system if needed:
 *
 *	MSX1:		define it as nop
 *	MSX2;		leave empty
 *	MTX:		define it as a  "ld e,#0" or similar
 *	RC2014:		at standard speed you need 34 cycles, a pair
 *			of EX (SP),HL gives you that and four clocks of
 *			headroom for 8MHz or so boxes.
 *
 *	For text modes only
 *	MSX1/MSX2:	leave empty
 *	MTX:		leave empty
 *	RC2014:		leave empty (just about - 8MHz will need a nop)
 *			RC2014 with a VDP9938/58 will need 16 clocks eg
 *			JR 0, NOP
 */

#include <v99xx.h>
#include <kernel.h>

#ifndef PLATFORM_VDP_DELAY
#define PLATFORM_VDP_DELAY
#endif

/* v99xx doesn't support register read
 * need to keep a local copy to change bits */
static uint8_t v99xx_regs[V99xx_NREGS];

void v99xx_write_reg(uint8_t reg, uint8_t val)
{
	v99xx_regs[reg]=val;
	__asm
	pop hl
	pop de
	push de
	push hl
	ld bc, (_vdpport)
	out (c), d
	ld a,e
	or #0x80
	out (c), a
	__endasm;
}

/*
 * v99xx can address up to 128Kb of VRAM
 * via page selection (up to 8 pages) in register #14
 */
void v99xx_set_vram_page(uint8_t page)
{
	v99xx_write_reg(V99xx_SET_VRAM_PAGE, page & 0x7);
}

/*
 * write byte in current ram page
 * (only 14 bits of address are used)
 */
void v99xx_write_vram(uint16_t addr, uint8_t val)
{
	used(addr);
	used(val);

	__asm
	pop bc
	pop hl
	pop de
	push de
	push hl
	push bc
	ld bc, (_vdpport)
	ld b,e
	out (c),l
	ld a, h
	or #0x40
	out (c),a
	dec c
	out (c),b
	__endasm;
}

/*
 * read byte from current ram page
 * (only 14 bits of address are used)
 */
uint8_t v99xx_read_vram(uint16_t addr)
{
	used(addr);
	__asm
	pop bc
	pop hl
	push hl
	push bc
	ld bc, (_vdpport)
	out (c),l
	out (c),h
	dec c
	in a,(c)
	__endasm;
	return 0;
}

/*
 * memset to vram current page (up to 16Kb)
 */
void v99xx_memset_vram(uint16_t addr, uint8_t value, uint16_t size)
{
	used(addr);
	used(value);
	used(size);

	__asm
	pop ix
	pop hl
	pop de
	dec sp
	ld bc, (_vdpport)
	ld b,e
	pop de
	push bc
	push de
	inc sp
	push hl
	push ix
	out (c),l
	ld a, h
	or #0x40
	out (c),a
	dec c
	;
	;	TODO: could be a little bit faster using djnz loops
	;
	ld a,b
	ld b,e
	dec de
	inc d
	;
	;	This loop takes 25 cycles, E is free for timing delays
	;
memset_loop:
	out (c),a
	PLATFORM_VDP_DELAY
	djnz memset_loop
	dec d
	jr nz,memset_loop
	__endasm;

}

/*
 * Copy from vram current page (up to 16Kb)
 */
void v99xx_copy_from_vram(uint8_t *dst, uint16_t vaddr, uint16_t size)
{
	used(vaddr);
	used(dst);
	used(size);

	__asm
	pop iy
	pop hl
	pop de
	ld bc, (_vdpport)
	out (c),e
	ld a,d
	out (c),d
	pop de
	push de
	push de
	push hl
	push iy
	dec c

	ld b,e	; Turn a 16 bit count into a number of repeats of an 8bit
	dec de	; count where 0 means 256
	inc d

	cp #0x3f
	jp m,cpfvram_loop

	in a,(c)
cpfvram_loop:
	;
	;	This loop must take 8us for worst case (TMS9918A Graphics 1)
	;	or 29 cycles on MSX1. The loop itself takes 26, E is free
	;	for timing delays
	;
	ini
	PLATFORM_VDP_DELAY
	jp nz, cpfvram_loop
	dec d
	jp nz,cpfvram_loop	; faster when taken than jr
	__endasm;
}

/*
 * Copies to vram current page (up to 16Kb)
 */
void v99xx_copy_to_vram(uint16_t vaddr, uint8_t *src, uint16_t size)
{
	used(vaddr);
	used(src);
	used(size);

	__asm
	pop iy
	pop de
	ld bc, (_vdpport)
	out (c),e
	ld a,d
	or #0x40
	out (c),a
	pop hl
	pop de
	push de
	push hl
	push de
	push iy
	dec c
	ld b,e
	dec de
	inc d
cptvram_loop:
	;
	;	This loop must take 8us for worst case (TMS9918A Graphics 1)
	;	or 29 cycles on MSX1. The loop itself takes 26, E is free
	;	for timing delays
	;
	outi
	PLATFORM_VDP_DELAY
	jp nz, cptvram_loop
	dec d
	jp nz,cptvram_loop
	__endasm;
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
