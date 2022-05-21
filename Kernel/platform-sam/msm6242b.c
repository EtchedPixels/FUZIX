/*
 *	RTC driver for the OKI MSM6242B (aka the SamDOS clock)
 */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <rtc.h>
#include <msm6242b.h>

static uint8_t rtc_buf[6];
uint8_t samrtc;

uint_fast8_t plt_rtc_secs(void)
{
    uint8_t r, v;

    if (!samrtc)
        return 0xFF;
    
    do {
        r = samrtc_in(0);
        v = samrtc_in(0x10);
    } while (r != samrtc_in(0));
    
    return v * 10 + r;
}

static void read_clock_once(void)
{
    uint8_t *p = rtc_buf;
    uint8_t n = 0;
    
    while(n < 0xC0) {
        *p++ = samrtc_in(n) | (samrtc_in(n + 0x10) << 4);
        n += 0x20;
    }
}

static void read_clock(void)
{
    uint8_t r;

    do {
        r = samrtc_in(0);
        read_clock_once();
    } while(samrtc_in(0) != r);
}

int plt_rtc_read(void)
{
	uint16_t len = sizeof(struct cmos_rtc);
	uint16_t y;
	struct cmos_rtc cmos;
	uint8_t *p = cmos.data.bytes;

	if (!samrtc) {
	    kputs("nortc\n");
	    udata.u_error = -EOPNOTSUPP;
	    return -1;
        }
	if (udata.u_count < len)
		len = udata.u_count;

        read_clock();

	y = rtc_buf[5];
	/* Year is the low 2 digits. We work on the basis that it's past
	   2000 now so just assume 20xx */
	*p++ = 0x20;
	*p++ = y;
	*p++ = rtc_buf[4] - 1;	/* month (convert from 1 based) */
	*p++ = rtc_buf[3];	/* day */
	*p++ = rtc_buf[2];	/* Hour */
	*p++ = rtc_buf[1];	/* Minute */
	*p = rtc_buf[0];	/* Second */
	cmos.type = CMOS_RTC_BCD;
	if (uput(&cmos, udata.u_base, len) == -1)
		return -1;
	return len;
}

/* For now */
int plt_rtc_write(void)
{
	udata.u_error = -EOPNOTSUPP;
	return -1;
}

uint8_t plt_rtc_probe(void)
{
        uint8_t r;

        if (samrtc)
            return 1;

        for (r = 0; r < 12; r++) {
            if (samrtc_in(r) > 9)
                return 0;
        }
        
        /* Now play with 24 hour mode */
        r = samrtc_in(0xF0);
        r &= ~4;
        samrtc_out(0xF0 | (r << 8));
        if (samrtc_in(0xF0) != r)
            return 0;
        r |= 4;	/* 24 hour */
        samrtc_out(0xF0 | (r << 8));
        if (samrtc_in(0xF0) != r)
            return 0;
        /* Looks like we have a valid Sam RTC */
        samrtc = 1;
        return 1;
}
