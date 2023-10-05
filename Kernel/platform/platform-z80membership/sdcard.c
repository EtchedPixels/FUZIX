#include <kernel.h>
#include <sdcard.h>
#include <devsd.h>
#include <blkdev.h>

__sfr __at 0xC4 spi_cs;
__sfr __at 0xC0 spi_clk;

void sd_spi_raise_cs(void)
{
    spi_cs = 1;
    spi_clk = 1;
}

void sd_spi_lower_cs(void)
{
    spi_cs = 0;
    spi_clk = 1;
}

void sd_spi_clock(bool fast) __z88dk_fastcall
{
    used(fast);
}

COMMON_MEMORY

bool sd_spi_receive_sector(void) __naked
{
  __asm
    ld a, (_blk_op+BLKPARAM_IS_USER_OFFSET)
    ld hl, (_blk_op+BLKPARAM_ADDR_OFFSET)
    or a
    push af
    call nz,map_proc_always
    call _sd_spi_rx_sector
    pop af
    ret z
    jp map_kernel
  __endasm;
}

bool sd_spi_transmit_sector(void) __naked
{
  __asm
    ld a, (_blk_op+BLKPARAM_IS_USER_OFFSET)
    ld hl, (_blk_op+BLKPARAM_ADDR_OFFSET)
    or a
    push af
    call nz,map_proc_always
    call _sd_spi_tx_sector
    pop af
    ret z
    jp map_kernel
  __endasm;
}

