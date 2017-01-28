/*-----------------------------------------------------------------------*/
/* DS1202 and DS1302 Serial Real Time Clock driver                       */
/* 2014-12-30 Will Sowerbutts                                            */
/*-----------------------------------------------------------------------*/

#define _DS1302_PRIVATE

#include <kernel.h>
#include <kdata.h>
#include <stdbool.h>
#include <printf.h>
#include <ds1302.h>

/****************************************************************************/
/* Code in this file is used only once, at startup, so we want it to live   */
/* in the DISCARD segment. sdcc only allows us to specify one segment for   */
/* each source file.                                                        */
/****************************************************************************/

/* we avoid using 32x32 bit multiply ds1302_read_rtc, because it causes sdcc to pull 
   in a huge __mullong helper -- which is nearly 500 bytes! */
static uint32_t multiply_8x32(uint8_t a, uint32_t b) __naked /* return a * b */
{
    __asm
        ; WRS -- simple 8x32 multiply routine
        ; low 16 bits in main registers, high 16 bits in alternate registers
        ; DE contains b, shifts left, HL contains accumulated result, A contains a, shifts right.
        ld iy, #0       ; load parameters from stack
        add iy, sp
        ld a, 2(iy)     ; load a
        ld e, 3(iy)     ; load low 16 bits of b
        ld d, 4(iy)
        and a           ; clear carry flag
        sbc hl, hl      ; low result = 0
        exx
        sbc hl, hl      ; high result = 0
        ld e, 5(iy)     ; load high 16 bits of b
        ld d, 6(iy)
        exx
mulgo:                  ; loop executes at most 8 times (depends on highest bit set in a)
        or a            ; test value of A
        jr z, muldone   ; if A is now zero we are done
        rra             ; shift A right one bit, rotate low bit into carry flag
        jr nc, mulnext  ; if low bit was zero, skip the accumulate
        add hl, de      ; add low 16 bits
        exx
        adc hl, de      ; add high 16 bits
        exx
mulnext:
        and a           ; clear carry flag
        rl e            ; double low 16 bits
        rl d
        exx
        rl e            ; double high 16 bits
        rl d
        exx
        jr mulgo        ; go again
muldone:
        exx             ; get the high 16 bits into DE
        push hl
        exx
        pop de
        ret             ; return with 32-bit result in DEHL, as per sdcc callng convention
    __endasm;
    a; b;  /* squelch compiler warning */
}

static const uint16_t mktime_moffset[12] = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };

void ds1302_write_register(uint8_t reg, uint8_t val)
{
    ds1302_set_pin_ce(true);
    ds1302_send_byte(reg);
    ds1302_send_byte(val);
    ds1302_set_pin_ce(false);
    ds1302_set_pin_clk(false);
}

void ds1302_write_seconds(uint8_t seconds)
{
    irqflags_t irq = di();
    ds1302_write_register(0x8E, 0x00);    /* write to control register: disable write-protect */
    ds1302_write_register(0x80, seconds); /* write to seconds register (bit 7 set: halts clock) */
    ds1302_write_register(0x8E, 0x80);    /* write to control register: enable write-protect */
    irqrestore(irq);
}

uint32_t ds1302_read_rtc(void)
{
    uint32_t ret;
    uint8_t buffer[7];
    uint8_t year, month, day, hour, minute, second;

    ds1302_read_clock(buffer, 7); /* read all calendar data */

    year   = uint8_from_bcd(buffer[6]);
    month  = uint8_from_bcd(buffer[4] & 0x1F);
    day    = uint8_from_bcd(buffer[3] & 0x3F);
    if(buffer[2] & 0x80) /* AM/PM 12-hour mode */
        hour   = uint8_from_bcd(buffer[2] & 0x1F) + (buffer[2] & 0x20 ? 12 : 0);
    else /* sensible 24-hour mode */
        hour   = uint8_from_bcd(buffer[2] & 0x3F);
    minute = uint8_from_bcd(buffer[1]);
    second = uint8_from_bcd(buffer[0] & 0x7F);

    if(buffer[0] & 0x80){ /* is the clock halted? */
        kputs("ds1302: start clock\n");
        ds1302_write_seconds(second); /* start it */
    }

    if(year < 70)
        year += 100;

    /* following code is based on utc_mktime() from ELKS 
       https://github.com/jbruchon/elks/blob/master/elkscmd/sh_utils/date.c */

    /* uses zero-based month index */
    month--;

    /* calculate days from years */
    ret = multiply_8x32(year - 70, 365);

    /* count leap days in preceding years */
    ret += ((year - 69) >> 2);

    /* calculate days from months */
    ret += mktime_moffset[month];

    /* add in this year's leap day, if any */
    if (((year & 3) == 0) && (month > 1)) {
        ret ++;
    }

    /* add in days in this month */
    ret += (day - 1);

    /* convert to hours */
    ret = multiply_8x32(24, ret);
    ret += hour;

    /* convert to minutes */
    ret = multiply_8x32(60, ret);
    ret += minute;

    /* convert to seconds */
    ret = multiply_8x32(60, ret);
    ret += second;

    /* return the result */
    return ret;
}

void ds1302_init(void)
{
    time_t tod;

    /* initialise the hardware into a sensible state */
    ds1302_set_pin_data_driven(true);
    ds1302_set_pin_data(false);
    ds1302_set_pin_ce(false);
    ds1302_set_pin_clk(false);

    tod.high = 0;                   /* until 2106 */
    tod.low = ds1302_read_rtc();
    wrtime(&tod);
}
