#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <timer.h>
#include <stdbool.h>
#include "config.h"
#include <z180.h>
#include <blkdev.h>

__sfr __at 0xA0 spi;
uint8_t spi_bits = 0xFC;

#define SPI_DATA	0x01
#define SPI_CLOCK	0x02
#define SPI_CS_RTC	0x04	/* \CS */
#define SPI_CS_SD	0x08	/* \CS */
#define SPI_CS_7SEG	0x10	/* \CS */

void sd_spi_clock(bool go_fast)
{
/* We don't care about speed - it's all software driven */
}

void sd_spi_raise_cs(void)
{
 spi_bits |= SPI_CS_RTC|SPI_CS_7SEG;
 spi_bits &= ~SPI_CS_SD;
 spi = spi_bits;
}

void sd_spi_lower_cs(void)
{
 spi_bits |= SPI_CS_SD;
 spi = spi_bits;
}

COMMON_MEMORY

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
    call _sd_spi_rx_sector
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
    call _sd_spi_tx_sector
    pop af
    or a
    jp nz,map_kernel
    ret
  __endasm;
}

