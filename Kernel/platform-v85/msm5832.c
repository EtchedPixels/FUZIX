/*
 *	RTC driver for the OKI MSM 5832 mapped Dual systems style. Much like
 *	the Trash 80 clock but indirectly mapped and with auto-hold.
 */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <rtc.h>

extern uint_fast8_t rtc_get(uint8_t port);

uint_fast8_t platform_rtc_secs(void)
{
    irqflags_t irqflags = di();
    uint_fast8_t secs = rtc_get(0);
    irqrestore(irqflags);
    return secs;
}

int platform_rtc_read(void)
{
    uint16_t len = sizeof(struct cmos_rtc);
    irqflags_t irqflags;
    struct cmos_rtc cmos;
    uint8_t *p;
    uint_fast8_t r, y;

    if (udata.u_count < len)
        len = udata.u_count;

    irqflags = di();

    p = cmos.data.bytes;
    y  = rtc_get(11);
    if (y >= 0x70)
        *p++ = 0x19;
    else
        *p++ = 0x20;
    *p++ = y;
    *p++ = rtc_get(9) & 0x1F;
    *p++ = rtc_get(7) & 0x3F;
    *p++ = rtc_get(4) & 0x3F;
    *p++ = rtc_get(2) & 0x7F;
    *p++ = rtc_get(0) & 0x7F;
    irqrestore(irqflags);

    cmos.type = CMOS_RTC_BCD;
    if (uput(&cmos, udata.u_base, len) == -1)
        return -1;
    return len;
}

/* TODO */
int platform_rtc_write(void)
{
	udata.u_error = EOPNOTSUPP;
	return -1;
}
