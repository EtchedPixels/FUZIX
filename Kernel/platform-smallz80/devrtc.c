#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <rtc.h>

__sfr __at 0x20 rtc0;
__sfr __at 0x21 rtc1;
__sfr __at 0x22 rtc2;
__sfr __at 0x23 rtc3;
__sfr __at 0x24 rtc4;
__sfr __at 0x25 rtc5;
__sfr __at 0x26 rtc6;
__sfr __at 0x27 rtc7;
__sfr __at 0x28 rtc8;
__sfr __at 0x29 rtc9;
__sfr __at 0x2A rtcA;
__sfr __at 0x2B rtcB;
__sfr __at 0x2C rtcC;
__sfr __at 0x2D rtcD;
__sfr __at 0x2E rtcE;
__sfr __at 0x2F rtcF;

/* Full RTC support (for read - no write yet) */
int plt_rtc_read(void)
{
        irqflags_t irq;
	uint16_t len = sizeof(struct cmos_rtc);
	struct cmos_rtc cmos;
	uint8_t *p = cmos.data.bytes;

	if (udata.u_count < len)
		len = udata.u_count;

        irq = di();
        rtcD |= 0x01;		/* Set hold and then */
        while(rtcD & 0x02);	/* spin until not busy */
        
        /* Now safe to read the clock */
        /* FIXME: we assume 24hr mode */
        p[6] = rtc0 | (rtc1 << 4);
        p[5] = rtc2 | (rtc3 << 4);
        p[4] = rtc4 | ((rtc5 << 4) & 0x30);
        p[3] = rtc6 | (rtc7 << 4);
        p[2] = rtc8 | (rtc9 << 4);
        p[1] = rtcA | (rtcB << 4);
        /* Assume 2000 based for now FIXME */
        p[0] = 0x20;

        rtcD &= ~1;		/* Hold off */

        irqrestore(irq);
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

uint8_t plt_rtc_secs(void)
{
        irqflags_t irq;
        uint8_t s;
        irq = di();
        rtcD |= 0x01;		/* Set hold and then */
        while(rtcD & 0x02);	/* spin until not busy */
        s = rtc0 + 10 * rtc1;
        rtcD &= ~0x01;
        irqrestore(irq);
        return s;
}
