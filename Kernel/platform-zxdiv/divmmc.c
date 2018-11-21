#include <kernel.h>
#include <blkdev.h>
#include <devsd.h>

__sfr __at 0xE7 divmmc_cs;
__sfr __at 0xEB divmmc_data;

void sd_spi_raise_cs(void)
{
    divmmc_cs = 0x03;		/* Active low */
}

void sd_spi_transmit_byte(uint8_t b)
{
    divmmc_data = b;
}

uint8_t sd_spi_receive_byte(void)
{
    return divmmc_data;
}

void sd_spi_lower_cs(void)
{
    divmmc_data = 0x02;			/* FIXME we can have a slave */
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
    or a	; Set the Z flag up and save it, dont do it twice
    push af
    call nz,map_process_always
    ld bc, #0xEB	 ; b = 0, c = port
    inir
    inir
    pop af
    call nz,map_kernel
    ret
  __endasm;
}

bool sd_spi_transmit_sector(void) __naked
{
  __asm
    ld a, (_blk_op+BLKPARAM_IS_USER_OFFSET)
    ld hl, (_blk_op+BLKPARAM_ADDR_OFFSET)
    or a	; Set the Z flag up and save it, dont do it twice
    push af
    call nz,map_process_always
    ld bc, #0xEB
    otir
    otir
    pop af
    call nz,map_kernel
    ret
  __endasm;
}

