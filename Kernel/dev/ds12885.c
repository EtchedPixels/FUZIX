/*
 *	PC style 12885/12887 and similar real time clock/NVRAM
 *
 *	One weirdness here is that the century feature only works in BCD mode
 */

#include <kernel.h>
#include <kdata.h>
#include <rtc.h>
#include <ds12885.h>
#include <printf.h>

#ifdef CONFIG_RTC_DS12885

uint_fast8_t ds12885_present;


/* The host provides nothing but a read and write reg function */

static uint_fast8_t nvmap(uint_fast8_t r)
{
    /* Skip century byte */
    r += 0x0E;
    if (r > 0x31)
        r++;
    return r;
}

static uint_fast8_t rtc_nvread(uint_fast8_t r)
{
    irqflags_t irq = di();
    uint_fast8_t res = ds12885_read(nvmap(r));
    irqrestore(irq);
    return res;
}

static void rtc_nvwrite(uint_fast8_t r, uint_fast8_t v)
{
    irqflags_t irq = di();
    ds12885_write(nvmap(r), v);
    irqrestore(irq);
}

int plt_rtc_ioctl(uarg_t request, char *data)
{
    struct cmos_nvram *rtc = (struct cmos_nvram *)data;
    uint16_t r;
    int v;

    /* We lose a byte to the century */
    if (request == RTCIO_NVSIZE)
        return 113;

    r = ugetw(&rtc->offset);
    if (r > 30) {
        udata.u_error = ERANGE;
        return -1;
    }
    switch(request) {
    case RTCIO_NVGET:
        return uputc(rtc_nvread(r), &rtc->val);
    case RTCIO_NVSET:
        v = ugetc(&rtc->val);
        if (v < 0)
            return -1;
        rtc_nvwrite(r, v);
        return 0;
    /* FIXME: do we need an alarm API ? */
    default:
        return -1;
    }
}

/* define CONFIG_RTC in platform's config.h to hook this into timer.c */

uint_fast8_t plt_rtc_secs(void)
{
    uint_fast8_t v = ds12885_read(DS12885_SEC);
    /* Turn it native */
    return (v & 0x0F) + (((v & 0xF0) >> 4) * 10);
}

int ds12885_battery_good(void)
{
    uint_fast8_t r= ds12885_read(DS12885_REGD);
    if (r & VRT)
        return 1;
    kputs("ds12885: battery low\n");
    return 0;
}

int plt_rtc_read(void)
{
	uint16_t len = sizeof(struct cmos_rtc);
	uint_fast8_t y;
	struct cmos_rtc cmos;
	uint8_t *p = cmos.data.bytes;
	irqflags_t irq;

	if (!ds12885_present) {
		udata.u_error = EOPNOTSUPP;
		return -1;
	}

        ds12885_battery_good();
	if (udata.u_count < len)
		len = udata.u_count;


        irq = di();
        /* Wait for a read window */
        while(ds12885_read(DS12885_REGA) & UIP);

        y = ds12885_read(DS12885_CEN);
        if (y == 0)
            y = 0x20;
	*p++ = y;
	*p++ = ds12885_read(DS12885_YR);
	*p++ = ds12885_read(DS12885_MON) - 1;   /* convert to 0-based month */
	*p++ = ds12885_read(DS12885_DOM);
	*p++ = ds12885_read(DS12885_HR);
	*p++ = ds12885_read(DS12885_MIN);
	*p = ds12885_read(DS12885_SEC);
	irqrestore(irq);
	cmos.type = CMOS_RTC_BCD;
	if (uput(&cmos, udata.u_base, len) == -1)
		return -1;
	return len;
}

int plt_rtc_write(void)
{
	uint16_t len = sizeof(struct cmos_rtc);
	struct cmos_rtc cmos;
	uint8_t *p = cmos.data.bytes;
	irqflags_t irq;

	if (!ds12885_present) {
		udata.u_error = EOPNOTSUPP;
		return -1;
	}

	if (udata.u_count != len) {
	    udata.u_error = EINVAL;
	    return -1;
        }
	if (uget(udata.u_base, &cmos, len) == -1)
		return -1;

        if (cmos.type != CMOS_RTC_BCD) {
            udata.u_error = EINVAL;
            return -1;
        }

        ds12885_battery_good();
        irq = di();
        /* Turn the SET bit on */
        ds12885_write(DS12885_REGB, ds12885_read(DS12885_REGB)|SET);
        ds12885_write(DS12885_CEN, *p++);
        ds12885_write(DS12885_YR,  *p++);
	ds12885_write(DS12885_MON, *p++ + 1);  /* convert from 0-based month */
        ds12885_write(DS12885_DOM, *p++);
        ds12885_write(DS12885_HR,  *p++);
        ds12885_write(DS12885_MIN, *p++);
        ds12885_write(DS12885_SEC, *p);
        /* Enable counting, preserve rate selector */
        ds12885_write(DS12885_REGA, DV1 | (ds12885_read(DS12885_REGA)&0x0F));
        /* Turn the SET bit off*/
        ds12885_write(DS12885_REGB, ds12885_read(DS12885_REGB) & ~SET);
        irqrestore(irq);
        return len;        
}

void ds12885_init(void)
{
        uint8_t chk = ds12885_read(DS12885_DOW);
        if (chk & 0xF8)
            return;
        chk = ds12885_read(DS12885_REGD);
        if (chk & 0x7F)
            return;
        chk = ds12885_read(DS12885_REGC);
        if (chk & 0x0F)
            return;
	/* Enable counting, preserve rate selector */
	ds12885_write(DS12885_REGA, DV1 | (ds12885_read(DS12885_REGA)&0x0F));
	/* Set BCD mode, 24hr, without hardware DSE */
	ds12885_write(DS12885_REGB, HR24);
	ds12885_battery_good();
	ds12885_present = 1;
}

uint_fast8_t ds12885_interrupt(void)
{
    return ds12885_read(DS12885_REGC);
}

/* Set the DS12885 up as an interval timer at 64Hz */
void ds12885_set_interval(void)
{
	/* Set rate selector for 64 Hz, make sure counter is on */
	ds12885_write(DS12885_REGA, 0x0A | DV1);
	/* Periodic interrupt enable */
	ds12885_write(DS12885_REGB, ds12885_read(DS12885_REGB) | PIE);
}

void ds12885_disable_interval(void)
{
	ds12885_write(DS12885_REGB, ds12885_read(DS12885_REGB) & ~PIE);
}

#endif
