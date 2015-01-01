/*-----------------------------------------------------------------------*/
/* DS1202 and DS1302 Serial Real Time Clock driver                       */
/* 2014-12-30 Will Sowerbutts                                            */
/*-----------------------------------------------------------------------*/

#include <kernel.h>
#include <kdata.h>
#include <stdbool.h>
#include <printf.h>
#include <ds1302.h>

static void ds1302_send_byte(uint8_t byte)
{
    uint8_t i;

#ifdef DS1302_DEBUG
    kprintf("ds1302: send byte 0x%x\n", byte);
#endif
    /* drive the data pin */
    ds1302_set_pin_data_driven(true);

    /* clock out one byte, LSB first */
    for(i=0; i<8; i++){
        ds1302_set_pin_clk(false);
        /* for data input to the chip the data must be valid on the rising edge of the clock */
        ds1302_set_pin_data(byte & 1);
        byte >>= 1;
        ds1302_set_pin_clk(true);
    }
}

static uint8_t ds1302_receive_byte(void)
{
    uint8_t i, b;

    /* tri-state the data pin */
    ds1302_set_pin_data_driven(false);

    /* clock in one byte, LSB first */
    b = 0;
    for(i=0; i<8; i++){
        ds1302_set_pin_clk(false);
        b >>= 1;
        /* data output from the chip is presented on the falling edge of each clock */
        /* note that output pin goes high-impedance on the rising edge of each clock */
        if(ds1302_get_pin_data())
            b |= 0x80;
        ds1302_set_pin_clk(true);
    }

    return b;
}

static uint8_t from_bcd(uint8_t value)
{
    return (value & 0x0F) + (10 * (value >> 4));
}

void ds1302_read_clock(uint8_t *buffer, uint8_t length)
{
    uint8_t i;
    irqflags_t irq = di();

    ds1302_set_pin_ce(true);
    ds1302_send_byte(0x81 | 0x3E); /* burst read all calendar data */
    for(i=0; i<length; i++){
        buffer[i] = ds1302_receive_byte();
#ifdef DS1302_DEBUG
        kprintf("ds1302: received byte 0x%x index %d\n", buffer[i], i);
#endif
    }
    ds1302_set_pin_ce(false);
    ds1302_set_pin_clk(false);
    irqrestore(irq);
}

/* define CONFIG_RTC in platform's config.h to hook this into timer.c */
uint8_t rtc_secs(void)
{
    uint8_t buffer;
    ds1302_read_clock(&buffer, 1);   /* read out only the seconds value */
    return from_bcd(buffer & 0x7F); /* mask off top bit (clock-halt) */
}

/****************************************************************************/
/* Code below this point used only once, at startup, so we want it to live  */
/* in the DISCARD segment. sdcc only allows us to specify one segment for   */
/* each source file. This "solution" is a bit (well, very) hacky ...        */
/****************************************************************************/
static void DISCARDSEG(void) __naked { __asm .area _DISCARD __endasm; }

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

uint32_t ds1302_read_rtc(void)
{
    uint32_t ret;
    uint8_t buffer[7];
    uint8_t year, month, day, hour, minute, second;

    ds1302_read_clock(buffer, 7); /* read all calendar data */

    year   = from_bcd(buffer[6]);
    month  = from_bcd(buffer[4] & 0x1F);
    day    = from_bcd(buffer[3] & 0x3F);
    if(buffer[2] & 0x80) /* AM/PM 12-hour mode */
        hour   = from_bcd(buffer[2] & 0x1F) + (buffer[2] & 0x20 ? 12 : 0);
    else /* sensible 24-hour mode */
        hour   = from_bcd(buffer[2] & 0x3F);
    minute = from_bcd(buffer[1]);
    second = from_bcd(buffer[0] & 0x7F);

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
