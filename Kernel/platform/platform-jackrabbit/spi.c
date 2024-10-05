#include <kernel.h>
#include <rabbit.h>
#include <blkdev.h>

static uint8_t reverse[256] = {
    0,
    /* TODO */
};

void sd_spi_clock(bool go_fast)
{
    if (go_fast)
        rabbit_spi_fast();
    else
        rabbit_spi_slow();
}

void sd_spi_raise_cs(void)
{
    /* TODO pick a line */
}

void sd_spi_lower_cs(void)
{
    /* TODO pick a line */
}

void sd_spi_transmit_byte(uint8_t byte)
{
    rabbit_spi_tx(reverse[byte]);
}

uint8_t sd_spi_receive_byte(void)
{
    return reverse[rabbit_spi_rx()];
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
#ifdef SWAPDEV
    cp #2
    jr nz, not_swapin
    ld a,(_blk_op+BLKPARAM_SWAP_PAGE)
    call map_for_swap
    jr doread
not_swapin:
#endif
    or a
    jr z,doread
    call map_proc_always
doread:
    call rabbit_spi_rxblock
    jp map_kernel
  __endasm;
}

bool sd_spi_transmit_sector(void) __naked
{
  __asm
    ld a, (_blk_op+BLKPARAM_IS_USER_OFFSET)
    ld hl, (_blk_op+BLKPARAM_ADDR_OFFSET)
#ifdef SWAPDEV
    cp #2
    jr nz, not_swapout
    ld a, (_blk_op+BLKPARAM_SWAP_PAGE)
    call map_for_swap
    jr dowrite
not_swapout:
#endif
    or a
    jr z,dowrite
    call map_proc_always
dowrite:
    call rabbit_spi_txblock
    jp map_kernel
  __endasm;
}
