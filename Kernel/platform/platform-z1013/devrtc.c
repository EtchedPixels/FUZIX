#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <rtc.h>

#ifdef CONFIG_RTC

/* Use the specific RTC card if configured that way, otherwise use the GIDE
   option */
#ifdef CONFIG_RTC_70
__sfr __banked __at 0x70 rtc0;
__sfr __banked __at 0x71 rtc1;
__sfr __banked __at 0x72 rtc2;
__sfr __banked __at 0x73 rtc3;
__sfr __banked __at 0x74 rtc4;
__sfr __banked __at 0x75 rtc5;
__sfr __banked __at 0x76 rtc6;
__sfr __banked __at 0x77 rtc7;
__sfr __banked __at 0x78 rtc8;
__sfr __banked __at 0x79 rtc9;
__sfr __banked __at 0x7A rtcA;
__sfr __banked __at 0x7B rtcB;
__sfr __banked __at 0x7C rtcC;
__sfr __banked __at 0x7D rtcD;
__sfr __banked __at 0x7E rtcE;
__sfr __banked __at 0x7F rtcF;
#else
__sfr __banked __at 0x0045 rtc0;
__sfr __banked __at 0x0145 rtc1;
__sfr __banked __at 0x0245 rtc2;
__sfr __banked __at 0x0345 rtc3;
__sfr __banked __at 0x0445 rtc4;
__sfr __banked __at 0x0545 rtc5;
__sfr __banked __at 0x0645 rtc6;
__sfr __banked __at 0x0745 rtc7;
__sfr __banked __at 0x0845 rtc8;
__sfr __banked __at 0x0945 rtc9;
__sfr __banked __at 0x0A45 rtcA;
__sfr __banked __at 0x0B45 rtcB;
__sfr __banked __at 0x0C45 rtcC;
__sfr __banked __at 0x0D45 rtcD;
__sfr __banked __at 0x0E45 rtcE;
__sfr __banked __at 0x0F45 rtcF;
#endif

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

#endif
