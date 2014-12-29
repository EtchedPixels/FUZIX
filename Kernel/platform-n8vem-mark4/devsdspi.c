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

#if 0
/* WRS: measured byte transfer time as 16.88us with Z180 @ 36.864MHz */
bool sd_spi_receive_block(uint8_t drive, uint8_t *ptr, unsigned int length)
{
    while(length--){
        *(ptr++) = sd_spi_receive_byte(drive);
    }
    return true;
}
#else
/* WRS: measured byte transfer time as approx 5.66us with Z180 @ 36.864MHz,
   three times faster. Main change is to start the next receive operation 
   as soon as possible and overlap the loop housekeeping with the receive. */
bool sd_spi_receive_block(uint8_t drive, uint8_t *ptr, unsigned int length) __naked
{
    __asm
waitrx: 
        in0 a, (_CSIO_CNTR)     ; wait for any current transmit or receive operation to complete
        tst a, #0x30
        jr nz, waitrx
        set 5, a                ; set CSIO_CNTR_RE, enable receive operation for first byte
        out0 (_CSIO_CNTR), a
        ld h, a                 ; stash value for reuse later
        ; load parameters from the stack
        ld iy, #3
        add iy, sp
        ld e, 0(iy)             ; ptr -> DE
        ld d, 1(iy)
        ld c, 2(iy)             ; count -> BC
        ld b, 3(iy)
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
        ld l, #1                ; return true
        ret
    __endasm;
    drive; ptr; length; /* squelch compiler warnings */
}
#endif

#if 0
/* WRS: measured byte transfer time as 10.36us with Z180 @ 36.864MHz */
bool sd_spi_transmit_block(uint8_t drive, uint8_t *ptr, unsigned int length)
{
    register uint8_t c, b, e;
    drive; /* not used */

    e = (CSIO_CNTR & ~CSIO_CNTR_RE) | CSIO_CNTR_TE;

    while(length){
        b = reverse_byte(*ptr);

        /* wait for transmit to complete */
        do{
            c = CSIO_CNTR;
        }while(c & CSIO_CNTR_TE);

        /* write the byte and enable transmitter */
        CSIO_TRDR = b;
        CSIO_CNTR = e;

        /* next byte */
        ptr++;
        length--;
    }
    return true;
}
#else
/* WRS: measured byte transfer time as 5.64us with Z180 @ 36.864MHz */
bool sd_spi_transmit_block(uint8_t drive, uint8_t *ptr, unsigned int length) __naked
{
    __asm
        ; load parameters from the stack
        ld iy, #3
        add iy, sp
        ld e, 0(iy)             ; ptr -> DE
        ld d, 1(iy)
        ld l, 2(iy)             ; count -> HL
        ld h, 3(iy)
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
        ld l, #1                ; return true
        ret
    __endasm;
    drive; ptr; length; /* squelch compiler warnings */
}
#endif
