#include <kernel.h>
#include <blkdev.h>
#include <devsd.h>

/* FIXME: optimise by unrolling inir and otir 16 ways */

__sfr __at 0x1F zxmmc_cs;
__sfr __at 0x3F zxmmc_data;

void sd_spi_raise_cs(void)
{
    zxmmc_cs = 0xF7;		/* Active low NMI off */
}

void sd_spi_transmit_byte(uint8_t b)
{
    zxmmc_data = b;
}

uint8_t sd_spi_receive_byte(void)
{
    return zxmmc_data;
}

void sd_spi_lower_cs(void)
{
    if (sd_drive == 0)
      zxmmc_cs = 0xF6;	/* Lower bit 0 (active low) */
    else
      zxmmc_cs = 0xF5;	/* Lower bit 1 (active low) */
}

void sd_spi_clock(bool go_fast)
{
}

COMMON_MEMORY

/*
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
    call nz,map_process_always
doread:
    ld bc, #0x3f	 ; b = 0, c = port
    ld a,#0x05
    out (0xfe),a
    inir
    ld a,#0x02
    out (0xfe),a
    inir
    ld a,(_vtborder)
    out (0xfe),a
    in a,(0x3f)		; required according to CP/M driver
    nop
    nop
    in a,(0x3f)
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
    call nz,map_process_always
dowrite:
    ld bc, #0x3f
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

