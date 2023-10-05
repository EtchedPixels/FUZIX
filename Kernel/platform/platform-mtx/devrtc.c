/*
 *	MTXPlus RTC: DS12887 at 0x70/0x71
 */

#include <kernel.h>
#include <kdata.h>
#include <stdbool.h>
#include <printf.h>
#include <rtc.h>
#include <mtx.h>

__sfr __at 0x70	rtc_reg;
__sfr __at 0x71 rtc_data;

/* define CONFIG_RTC in platform's config.h to hook this into timer.c */
uint_fast8_t plt_rtc_secs(void)
{
	if (has_mtxplus) {
		irqflags_t irq = di();
		uint8_t r;
		rtc_reg = 0;
		r = rtc_data;
		irqrestore(irq);
		return r;
	}
	return 0xFF;
}

/* Full RTC support (for read - no write yet) */
int plt_rtc_read(void)
{
	uint16_t len = sizeof(struct cmos_rtc);
	irqflags_t irq;
	uint8_t m, s;
	struct cmos_rtc cmos;
	uint8_t *p = cmos.data.bytes;

	if (!has_mtxplus) {
		udata.u_error = EOPNOTSUPP;
		return -1;
	}
	if (udata.u_count < len)
		len = udata.u_count;

        irq = di();
        rtc_reg = 0;
        s = rtc_data;
        do {
	        rtc_reg = 9;
		*p++ = 0x20;
		*p++ = rtc_data;
		rtc_reg = 8;
		m = rtc_data - 1;	/* Month */
		/* Need to subtract 1 in BCD */
		if (--m == 0x0F)
			m = 0x09;
		*p++ = m;
		rtc_reg = 7;
		*p++ = rtc_data;	/* Day of month */
		rtc_reg = 4;
		*p++ = rtc_data;	/* Hour */
		rtc_reg = 2;
		*p++ = rtc_data;	/* Minute */
		rtc_reg = 0;
		*p = rtc_data;		/* Second */
	} while(*p != s);
	irqrestore(irq);
	/* Decimal fields */
	cmos.type = CMOS_RTC_BCD;
	if (uput(&cmos, udata.u_base, len) == -1)
		return -1;
	return len;
}

int plt_rtc_write(void)
{
	udata.u_error = EOPNOTSUPP;
	return -1;
}

void plt_rtc_init(void)
{
	if (has_mtxplus) {
		/* Set 24 hour mode */
		uint8_t d;
		rtc_reg = 0x0B;
		d = rtc_data | 2;
		if (rtc_data & 4)
			kputs("rtc: oops !BCD.\n");
		/* Turn off DSE */
		d &= ~1;
		rtc_data = d;
		rtc_reg = 0x0D;
		if (!(rtc_data & 0x80))
			kputs("rtc: low battery.\n");
	}
}
