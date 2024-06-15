/*-----------------------------------------------------------------------*/
/* N8 Z180 CSI/O SPI SD driver                                           */
/* Heavily based on the Mark4 driver					 */
/* 2014-12-27 Will Sowerbutts                                            */
/* 2014-12-29 Optimised for size/speed                                   */
/*-----------------------------------------------------------------------*/

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <timer.h>
#include <stdbool.h>
#include "config.h"
#include <z180.h>
#include <blkdev.h>

uint8_t rtc_shadow;			/* so we can stop the rtc messing us up */
__sfr __at 0x88 rtc_latch;

#define CSIO_CNTR_TE           (1<<4)   /* tx enable */
#define CSIO_CNTR_RE           (1<<5)   /* rx enable */
#define CSIO_CNTR_END_FLAG     (1<<7)   /* operation completed flag */

/* the CSI/O and SD card send the bits of each byte in opposite orders, so we need to flip them over */
static uint8_t reverse_byte(uint8_t byte) __naked
{
    /* code by John Metcalf, from http://www.retroprogramming.com/2014/01/fast-z80-bit-reversal.html */
    __asm
        ld hl, #2
        add hl, sp
        ld a, (hl)
reverse_byte_a:
        ; reverse bits in A
        ld l,a    ; a = 76543210
        rlca
        rlca      ; a = 54321076
        xor l
        and #0xAA
        xor l     ; a = 56341270
        ld l,a
        rlca
        rlca
        rlca      ; a = 41270563
        rrc l     ; l = 05634127
        xor l
        and #0x66
        xor l     ; a = 01234567
        ld l, a   ; return value in L
        ret
    __endasm;
    byte; /* squelch compiler warning */
}

void sd_spi_fast(void)
{
    unsigned char c;

    c = CSIO_CNTR & 0xf8; /* clear low three bits, gives fastest rate (clk/20) */
    CSIO_CNTR = c;
}

void sd_spi_slow(void)
{
    unsigned char c;

    c = CSIO_CNTR & 0xf8; /* clear low three bits, gives fastest rate (clk/20) */
    c = c | 0x03;     /* set low two bits, clk/160 (can go down to clk/1280, see data sheet) */
    CSIO_CNTR = c;
}

static void rtc_config(uint8_t m, uint8_t v)
{
    irqflags_t irq = di();
    rtc_shadow &= ~m;
    rtc_shadow |= v;
    rtc_latch = rtc_shadow;
    irqrestore(irq);
}

void sd_spi_raise_cs(void)
{
    /* wait for idle */
    while(CSIO_CNTR & (CSIO_CNTR_TE | CSIO_CNTR_RE));
    /* Raise CS */
    rtc_config(0x04, 0x00);
}

void sd_spi_lower_cs(void)
{
    /* wait for idle */
    while(CSIO_CNTR & (CSIO_CNTR_TE | CSIO_CNTR_RE));
    /* Lower CS */
    rtc_config(0x04, 0x04);
}

void sd_spi_tx_byte(unsigned char byte)
{
    unsigned char c;

    /* reverse the bits before we busywait */
    byte = reverse_byte(byte);

    /* wait for any current tx operation to complete */
    do{
        c = CSIO_CNTR;
    }while(c & CSIO_CNTR_TE);

    /* write the byte and enable txter */
    CSIO_TRDR = byte;
    CSIO_CNTR = c | CSIO_CNTR_TE;
}

uint8_t sd_spi_rx_byte(void)
{
    unsigned char c;

    /* wait for any current tx or rx operation to complete */
    do{
        c = CSIO_CNTR;
    }while(c & (CSIO_CNTR_TE | CSIO_CNTR_RE));

    /* enable rx operation */
    CSIO_CNTR = c | CSIO_CNTR_RE;

    /* wait for rx to complete */
    while(CSIO_CNTR & CSIO_CNTR_RE);

    /* read byte */
    return reverse_byte(CSIO_TRDR);
}

/****************************************************************************/
/* The innermost part of the transfer routines has to live in common memory */
/* since it must be able to bank switch to the user memory bank.            */
/****************************************************************************/
COMMON_MEMORY

/* WRS: measured byte transfer time as approx 5.66us with Z180 @ 36.864MHz,
   three times faster. Main change is to start the next rx operation 
   as soon as possible and overlap the loop housekeeping with the rx. */
bool sd_spi_rx_sector(uint8_t *data) __naked
{
    __asm
waitrx: 
        pop bc
        pop de
        push de			; address
        push bc
        in0 a, (_CSIO_CNTR)     ; wait for any current tx or rx operation to complete
        tst a, #0x30
        jr nz, waitrx
        set 5, a                ; set CSIO_CNTR_RE, enable rx operation for first byte
        out0 (_CSIO_CNTR), a
        ld h, a                 ; stash value for reuse later

        ; load parameters
        ld a, (_td_raw)
        ld bc, #512                             ; sector size
#ifdef SWAPDEV
        cp #2
        jr nz, not_swapin
        ld a,(_td_page)
        call map_for_swap
        jr rxnextbyte
not_swapin:
#endif
        or a
        jr z, rd_kernel
        call map_proc_always             ; map user process
        jr rxnextbyte
rd_kernel:
        call map_buffers
rxnextbyte:
        dec bc                  ; length--
        ld a, b
        or c
        jr nz, waitrx2
        res 5, h                ; final byte: clear CSIO_CNTR_RE bit in H
waitrx2:
        ld l, c                 ; store C temporarily
        ld c, #_CSIO_CNTR       ; load IO port address
waitrx3:
        tstio #0x20             ; test bits in IO port (C)
        jr nz, waitrx3          ; wait for rx to complete
        in0 a, (_CSIO_TRDR)     ; load rxd byte
        out0 (_CSIO_CNTR), h    ; start next rx (or NOP, if this is the final byte)
        ld c, l                 ; restore C
        ; reverse bits in A
        ld l,a    ; a = 76543210
        rlca
        rlca      ; a = 54321076
        xor l
        and #0xAA
        xor l     ; a = 56341270
        ld l,a
        rlca
        rlca
        rlca      ; a = 41270563
        rrc l     ; l = 05634127
        xor l
        and #0x66
        xor l     ; a = 01234567
        ld (de), a              ; store reversed byte value
        inc de                  ; ptr++
        bit 5, h
        jr nz, rxnextbyte       ; go again if not yet done
        jr transferdone         ; we are done
    __endasm;
}

bool sd_spi_tx_sector(uint8_t *data) __naked
{
    __asm
        ; load parameters
        pop bc
        pop de
        push de
        push bc
        ld a,(_td_raw)
        ld hl, #512                             ; sector size
#ifdef SWAPDEV
        cp #2
        jr nz, not_swapout
        ld a,(_td_page)
        call map_for_swap
        jr gotx
not_swapout:
#endif
        or a
        jr z, wr_kernel
        call map_proc_always             ; map user process
        jr gotx
wr_kernel:
        call map_buffers
gotx:
        in0 a, (_CSIO_CNTR)
        and #0xDF               ; mask off RE bit
        or #0x10                ; set TE bit
        ld b, a                 ; B now contains CNTR register value to start transmission
txnextbyte:
        ld a, (de)
        ; reverse bits in A
        ld c,a
        rlca
        rlca
        xor c
        and #0xAA
        xor c
        ld c,a
        rlca
        rlca
        rlca
        rrc c
        xor c
        and #0x66
        xor c
        ld c, #_CSIO_CNTR       ; load IO port address
waittx: 
        tstio #0x10             ; test bits in IO port (C)
        jr nz, waittx           ; wait for tx to complete
        out0 (_CSIO_TRDR), a    ; write byte to tx
        out0 (_CSIO_CNTR), b    ; start tx
        inc de                  ; ptr++
        dec hl                  ; length--
        ld a, h
        or l
        jr nz, txnextbyte       ; length != 0, go again
transferdone:                   ; note this code is shared with sd_spi_rx_block
        ld l, #1                ; return true
        jp map_kernel_restore   ; map kernel and return
    __endasm;
}
