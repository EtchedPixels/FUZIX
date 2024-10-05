/*
 *	ZX Evolution MMC.
 *
 *	TODO: Add support for the card present check and WP flag
 */
#include <kernel.h>
#include <blkdev.h>
#include <devsd.h>

__sfr __at 0x77 evo_cs;
__sfr __at 0x57 evo_data;

void sd_spi_raise_cs(void)
{
    evo_cs = 0xFF;		/* Active low */
}

void sd_spi_transmit_byte(uint8_t b)
{
    evo_data = b;
    evo_data;		/* and discard the read pair ? CHECK */
}

uint8_t sd_spi_receive_byte(void)
{
    return evo_data;
}

void sd_spi_lower_cs(void)
{
      evo_cs = 0xFD;	/* Lower bit 1 (active low) */
}

void sd_spi_clock(bool go_fast)
{
}

COMMON_MEMORY

/*
 * FIXME: swap support
 *
 * Could also unroll these a bit for speed
 */

bool sd_spi_receive_sector(void) __naked
{
  __asm
    ld a, (_blk_op+BLKPARAM_IS_USER_OFFSET)
    ld hl, (_blk_op+BLKPARAM_ADDR_OFFSET)
    push af
#ifdef SWAPDEV
    cp #2
    jr nz, not_swapin
    ld a,(_blk_op+BLKPARAM_SWAP_PAGE)
    call map_for_swap
    jr doread
not_swapin:
#endif
    or a
    call nz,map_proc_always
doread:
    ld bc, #0x57	 ; b = 0, c = port
    ld a,#0x05
    out (0xfe),a
    inir
    ld a,#0x02
    out (0xfe),a
    inir
    ld a,(_vtborder)
    out (0xfe),a
    pop af
    or a
    jp nz,map_kernel
    ret
  __endasm;
}

bool sd_spi_transmit_sector(void) __naked
{
  __asm
    ld a, (_blk_op+BLKPARAM_IS_USER_OFFSET)
    ld hl, (_blk_op+BLKPARAM_ADDR_OFFSET)
    push af
#ifdef SWAPDEV
    cp #2
    jr nz, not_swapout
    ld a, (_blk_op+BLKPARAM_SWAP_PAGE)
    call map_for_swap
    jr dowrite
not_swapout:
#endif
    or a
    call nz,map_proc_always
dowrite:
    ld bc, #0x57
    ld a,#0x05
    out (0xfe),a
    otir
    ld a,#0x02
    out (0xfe),a
    otir
    ld a,(_vtborder)
    out (0xfe),a
    pop af
    or a
    jp nz,map_kernel
    ret
  __endasm;
}

