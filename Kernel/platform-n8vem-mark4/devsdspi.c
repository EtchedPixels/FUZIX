/*-----------------------------------------------------------------------*/
/* N8VEM Mark IV Z180 CSI/O SPI SD driver                                */
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

#define CSIO_CNTR_TE           (1<<4)   /* transmit enable */
#define CSIO_CNTR_RE           (1<<5)   /* receive enable */
#define CSIO_CNTR_END_FLAG     (1<<7)   /* operation completed flag */

#define MARK4_SD_CS            (1<<2)   /* chip select */ 
#define MARK4_SD_WRITE_PROTECT (1<<4)   /* write protect */ 
#define MARK4_SD_CARD_DETECT   (1<<5)   /* card detect */
#define MARK4_SD_INT_ENABLE    (1<<6)   /* interrupt enable */
#define MARK4_SD_INT_PENDING   (1<<7)   /* interrupt enable */

__sfr __at (MARK4_IO_BASE + 0x09) MARK4_SD;

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

void sd_spi_clock(uint8_t drive, bool go_fast)
{
    unsigned char c;
    drive; /* not used */

    c = CSIO_CNTR & 0xf8; /* clear low three bits, gives fastest rate (clk/20) */
    if(!go_fast)
        c = c | 0x03;     /* set low two bits, clk/160 (can go down to clk/1280, see data sheet) */
    CSIO_CNTR = c;
}

void sd_spi_raise_cs(uint8_t drive)
{
    drive; /* not used */
    /* wait for idle */
    while(CSIO_CNTR & (CSIO_CNTR_TE | CSIO_CNTR_RE));
    MARK4_SD = MARK4_SD & (~MARK4_SD_CS);
}

void sd_spi_lower_cs(uint8_t drive)
{
    drive; /* not used */
    /* wait for idle */
    while(CSIO_CNTR & (CSIO_CNTR_TE | CSIO_CNTR_RE));
    MARK4_SD = MARK4_SD | MARK4_SD_CS;
}

void sd_spi_transmit_byte(uint8_t drive, unsigned char byte)
{
    unsigned char c;
    drive; /* not used */

    /* reverse the bits before we busywait */
    byte = reverse_byte(byte);

    /* wait for any current transmit operation to complete */
    do{
        c = CSIO_CNTR;
    }while(c & CSIO_CNTR_TE);

    /* write the byte and enable transmitter */
    CSIO_TRDR = byte;
    CSIO_CNTR = c | CSIO_CNTR_TE;
}

uint8_t sd_spi_receive_byte(uint8_t drive)
{
    unsigned char c;
    drive; /* not used */

    /* wait for any current transmit or receive operation to complete */
    do{
        c = CSIO_CNTR;
    }while(c & (CSIO_CNTR_TE | CSIO_CNTR_RE));

    /* enable receive operation */
    CSIO_CNTR = c | CSIO_CNTR_RE;

    /* wait for receive to complete */
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
   three times faster. Main change is to start the next receive operation 
   as soon as possible and overlap the loop housekeeping with the receive. */
bool sd_spi_receive_sector(uint8_t drive) __naked
{
    __asm
waitrx: 
        in0 a, (_CSIO_CNTR)     ; wait for any current transmit or receive operation to complete
        tst a, #0x30
        jr nz, waitrx
        set 5, a                ; set CSIO_CNTR_RE, enable receive operation for first byte
        out0 (_CSIO_CNTR), a
        ld h, a                 ; stash value for reuse later

        ; load parameters
        ld a, (_blk_op+BLKPARAM_IS_USER_OFFSET) ; blkparam.is_user
        ld de, (_blk_op+BLKPARAM_ADDR_OFFSET)   ; blkparam.addr
        ld bc, #512                             ; sector size
        or a
        push af                                 ; stash is_user flag now in Z bit (we cannot load it again after we remap)
        jr z, rxnextbyte
        call map_process_always                 ; map user process
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
        jr nz, waitrx3          ; wait for receive to complete
        in0 a, (_CSIO_TRDR)     ; load received byte
        out0 (_CSIO_CNTR), h    ; start next receive (or NOP, if this is the final byte)
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
    drive; /* squelch compiler warnings */
}

bool sd_spi_transmit_sector(uint8_t drive) __naked
{
    __asm
        ; load parameters
        ld a, (_blk_op+BLKPARAM_IS_USER_OFFSET) ; blkparam.is_user
        ld de, (_blk_op+BLKPARAM_ADDR_OFFSET)   ; blkparam.addr
        ld hl, #512                             ; sector size
        or a
        push af                                 ; stash is_user flag now in Z bit (we cannot load it again after we remap)
        jr z, gotransmit
        call map_process_always                 ; map user process
gotransmit:
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
        jr nz, waittx           ; wait for transmit to complete
        out0 (_CSIO_TRDR), a    ; write byte to transmit
        out0 (_CSIO_CNTR), b    ; start transmit
        inc de                  ; ptr++
        dec hl                  ; length--
        ld a, h
        or l
        jr nz, txnextbyte       ; length != 0, go again
transferdone:                   ; note this code is shared with sd_spi_receive_block
        ld l, #1                ; return true
        pop af                  ; recover is_user bit in Z flag
        ret z                   ; return if kernel still mapped
        jp map_kernel           ; else map kernel and return
    __endasm;
    drive; /* squelch compiler warnings */
}
