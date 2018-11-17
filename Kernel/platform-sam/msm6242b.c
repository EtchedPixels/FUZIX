/*
 *	RTC driver for the OKI MSM6242B (aka the SamDOS clock)
 */

#include <kernel.h>
#include <kdata.h>
#include <rtc.h>
#include <msm6242b.h>

static uint8_t rtc_buf[6];
static uint8_t samrtc = 0;

uint8_t platform_rtc_secs(void)
{
    uint8_t r, v;

    if (!samrtc)
        return 0xFF;
    
    do {
        r = samrtc_in(0);
        v = samrtc_in(1);
    } while (r != samrtc_in(0));
    
    return v * 10 + r;
}

static void read_clock_once(void)
{
    uint8_t *p = rtc_buf;
    uint8_t n = 0;
    
    while(n < 12) {
        *p++ = samrtc_in(n) | (samrtc_in(n + 1) << 4);
        n += 2;
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

int platform_rtc_read(void)
{
	uint16_t len = sizeof(struct cmos_rtc);
	uint16_t y;
	struct cmos_rtc cmos;
	uint8_t *p = cmos.data.bytes;

	if (!samrtc) {
	    udata.u_error = -EOPNOTSUPP;
	    return -1;
        }
	if (udata.u_count < len)
		len = udata.u_count;

        read_clock();

	y = rtc_buf[5];
	/* 1980 based year , if its 0x20 or more we are in 2000-2079 and
	   need to adjust the clock up by 2000 and back by 10. If not well
	   then its 1980-1999 so just needs 0x1980 adding to the 0-9 we have */
	if (y >= 0x20)
	    y += 0x19F0;
        else
            y += 0x1980;
	*p++ = y >> 8;
	*p++ = y;
	*p++ = rtc_buf[4];	/* month */
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
int platform_rtc_write(void)
{
	udata.u_error = -EOPNOTSUPP;
	return -1;
}

uint8_t platform_rtc_probe(void)
{
        uint8_t r;

        for (r = 0; r < 12; r++) {
            if (samrtc_in(r) > 9)
                return 0;
        }
        
        /* Now play with 24 hour mode */
        r = samrtc_in(15);
        r &= ~4;
        samrtc_out(15 | (r << 8));
        if (samrtc_in(15) != r)
            return 0;
        r |= 4;	/* 24 hour */
        samrtc_out(15 | (r << 8));
        if (samrtc_in(15) != r)
            return 0;
        /* Looks like we have a valid Sam RTC */
        samrtc = 1;
        return 1;
}
