/*
 *	This is intended for the NC200. It assumes 24 hour clock mode and
 *	decimal. The year is 1990 based. This is all fine for the NC200
 *	as that is how the firmware/OS leave it.
 */

#include <kernel.h>
#include <kdata.h>
#include <stdbool.h>
#include <printf.h>
#include <rtc.h>

__sfr __at 0xD0 rtc_second;
__sfr __at 0xD2 rtc_minute;
__sfr __at 0xD4 rtc_hour;
__sfr __at 0xD7 rtc_day;
__sfr __at 0xD8 rtc_month;
__sfr __at 0xD9 rtc_year;
__sfr __at 0xDA rtc_rega;

uint_fast8_t plt_rtc_secs(void)
{
        static uint8_t last;
        if (rtc_rega & 0x80)
            return last;
        return rtc_second;
}

/* Full RTC support (for read - no write yet) */
int plt_rtc_read(void)
{
	uint16_t len = sizeof(struct cmos_rtc);
	uint16_t y;
	irqflags_t flags;
	struct cmos_rtc cmos;
	uint8_t *p = cmos.data.bytes;

	if (udata.u_count < len)
		len = udata.u_count;

sync:
        while(rtc_rega & 0x80);	/* Wait for UIP to clear */
        
        flags = di();
        if (rtc_rega & 0x80)
            goto sync;

        /* We are now safe for 244uS */
	y = rtc_year + 1990;
	*p++ = y;
	*p++ = y >> 8;
	*p++ = rtc_month;
	*p++ = rtc_day;
        *p++ = rtc_hour;
        *p++ = rtc_minute;
        *p++ = rtc_second;
        irqrestore(flags);

	cmos.type = CMOS_RTC_DEC;
	if (uput(&cmos, udata.u_base, len) == -1)
		return -1;
	return len;
}

int plt_rtc_write(void)
{
	udata.u_error = -EOPNOTSUPP;
	return -1;
}
