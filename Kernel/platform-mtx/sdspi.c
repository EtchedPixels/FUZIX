#include <kernel.h>
#include <blkdev.h>
#include <devsd.h>

/* FIXME: optimise by unrolling inir and otir 16 ways */

__sfr __at 0xD4 sd_ctrl;
__sfr __at 0xD6 sd_io;		/* Rx/Tx */
__sfr __at 0xD7 sd_read;	/* Sends FF */

uint8_t has_rememo;

static uint8_t sd_clock;

void sd_spi_raise_cs(void)
{
    sd_ctrl = sd_clock;
}

void sd_spi_transmit_byte(uint8_t b)
{
    sd_io = b;
}

uint8_t sd_spi_receive_byte(void)
{
    return sd_read;	/* Will send 0xFF */
}

void sd_spi_lower_cs(void)
{
    sd_ctrl = sd_clock | 0x80;
}

void sd_spi_clock(bool go_fast)
{
    sd_clock = go_fast ? 0 : 0x04;
}

COMMON_MEMORY

/*
 * These have to match the bus timing. There is no \WAIT or anything else
 * to save us if we are too fast for the transfer.
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
    ld bc, #0xD7	; b = 0, c = port
rs1:
    ini			; 16T we need to burn 11
    jr nz, rs1		; burns 12 clocks if taken
    ; Need to burn 4 more clocks if not
    ; jp would be 10/10 not 11 so too fast
    nop
rs2:
    ini
    jr nz, rs2
    pop af		; Last byte is still in flight
    or a		;
    ret z		; But this path is always >= 27 clocks
    jp map_kernel
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
    ld bc, #0xD6
ws1:
    outi
    jr nz,ws1
    nop			; Same game as read
ws2:
    outi
    jr nz,ws2
    pop af		; and we dont have to worry about the end delay
    or a
    ret z
    jp map_kernel
  __endasm;
}

