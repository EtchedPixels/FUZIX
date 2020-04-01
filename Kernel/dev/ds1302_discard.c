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

void ds1302_write_seconds(uint_fast8_t seconds)
{
    irqflags_t irq = di();
    ds1302_write_register(0x8E, 0x00);    /* write to control register: disable write-protect */
    ds1302_write_register(0x80, seconds); /* write to seconds register (bit 7 set: halts clock) */
    ds1302_write_register(0x8E, 0x80);    /* write to control register: enable write-protect */
    irqrestore(irq);
}

static uint_fast8_t bad_bcd(uint8_t x, uint8_t min, uint8_t max)
{
    uint8_t c;

    c = x >> 4;
    x &= 15;
    if (c > 9 || x > 9)
        return 1;
    c = c * 10 + x;
    if (c < min || c > max)
        return 1;
    return 0;
}

uint_fast8_t ds1302_check_rtc(void)
{
    uint8_t buffer[7];

    ds1302_read_clock(buffer, 7); /* read all calendar data */

    if (bad_bcd(buffer[0] & 0x7F, 0, 59) ||
        bad_bcd(buffer[1], 0, 59) ||
        bad_bcd(buffer[3], 1, 31) ||
        bad_bcd(buffer[4], 1, 12) ||
        bad_bcd(buffer[5], 1, 7) ||
        bad_bcd(buffer[6], 0, 99))
            return 0;

    if(buffer[0] & 0x80){ /* is the clock halted? */
        kputs("ds1302: start clock\n");
        ds1302_write_seconds(buffer[0] & 0x7F); /* start it */
    }
    ds1302_read_clock(buffer, 7);
    if (buffer[0] & 0x80)
        return 0;
    return 1;
}

uint_fast8_t ds1302_init(void)
{
    /* initialise the hardware into a sensible state */
    ds1302_set_pin_data_driven(true);
    ds1302_set_pin_data(false);
    ds1302_set_pin_ce(false);
    ds1302_set_pin_clk(false);
    if (ds1302_check_rtc() == 0)
        return 0;
    ds1302_present = 1;
    return 1;
}
