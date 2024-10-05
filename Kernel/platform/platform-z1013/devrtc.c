#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <rtc.h>

#ifdef CONFIG_RTC

/* Use the specific RTC card if configured that way, otherwise use the GIDE
   option */
#ifdef CONFIG_RTC_70
#define rtc(x)		in(0x70 + (x))
#define wrtc(x,y)	out(0x70 + (x),(y))
#else
#define rtc(x)		in(0x45 + ((x) << 8))
#define wrtc(x,y)	out(0x45 + ((x) << 8), (y))
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
        wrtc(0xD, rtc(0xD) | 1);/* Set hold and then */
        while(rtc(0xD) & 0x02);	/* spin until not busy */
        
        /* Now safe to read the clock */
        /* FIXME: we assume 24hr mode */
        p[6] = rtc(0) | (rtc(1) << 4);
        p[5] = rtc(2) | (rtc(3) << 4);
        p[4] = rtc(4) | ((rtc(5) << 4) & 0x30);
        p[3] = rtc(6) | (rtc(7) << 4);
        p[2] = rtc(8) | (rtc(9) << 4);
        p[1] = rtc(0xA) | (rtc(0xB) << 4);
        /* Assume 2000 based for now FIXME */
        p[0] = 0x20;

	/* Hold off */
        wrtc(0xD, rtc(0xD) & ~1);

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

uint_fast8_t plt_rtc_secs(void)
{
        irqflags_t irq;
        uint8_t s;
        irq = di();
        wrtc(0xD, rtc(0xD) | 0x01);	/* Set hold and then */
        while(rtc(0xD) & 0x02);	/* spin until not busy */
        s = rtc(0) + 10 * rtc(1);
        wrtc(0xD, rtc(0xD) & ~1);
        irqrestore(irq);
        return s;
}

#endif
