/*
 *	PC style 12885/12887 and similar real time clock/NVRAM
 *
 *	One weirdness here is that the century feature only works in BCD mode
 */

#include <kernel.h>
#include <rtc.h>
#include <ds12885.h>


static uint_fast8_t ds12885_present;


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
    return ds12885_read(nvmap(r));
}

static void rtc_nvwrite(uint_fast8_t r, uint_fast8_t v)
{
    ds12885_read(nvmap(r), v);
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
    uint_fast8_t v = ds12885_read_bcd(0x00);
    /* Turn it native */
    return (v & 0x0F) | (((v & 0xF0) >> 4) * 10);
}

int ds12885_battery_good(void)
{
    uint_fast8_t r= ds12885_read(0x0D);
    if (r & 0x80)
        return 1;
    kprintf("ds12885: battery low.\n");
    return 0;
}

/* Full RTC support (for read - no write yet) */
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
        while(ds12885_read(0x0B) & 0x80);

        y = ds12885_read(0x32);
        if (y == 0)
            y = 0x20;
	*p++ = y;
	*p++ = ds12885_read(0x09);

	*p++ = ds12885_read(0x08) - 1;	/* 0 based month */
	*p++ = ds12885_read(0x07);	/* Day of month */
	*p++ = ds12885_read(0x04);
	*p++ = ds12885_read(0x02);
	*p = ds12885_read(0x00);
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

	if (!ds12885_present) {
		udata.u_error = EOPNOTSUPP;
		return -1;
	}

	if (udata.u_count != len) {
	    udata.u_error = EINVAL;
	    return -1;
        }
	if (uget(&cmos, udata.u_base, len) == -1)
		return -1;

        if (cmos.type != CMOS_RTC_BCD) {
            udata.u_error = EINVAL;
            return -1;
        }

        ds12885_battery_good();
        irq = di();
        ds12885_write(0x0B, ds12885_read(0x0B)| 0x80);
        ds12885_write(0x32, *p++);
        ds12885_write(0x09, *p++);
        ds12885_write(0x08, *p++);
        ds12885_write(0x07, *p++);
        ds12885_write(0x04, *p++);
        ds12885_write(0x02, *p++);
        ds12885_write(0x00, *p);
        /* Ensure counting */
        ds12885_write(0x0A, 0x20 | (ds12885_read(0x0A) & 0x0F));
        /* Turn the set function onff*/
        ds12885_write(0x0B, ds12885_read(0x0B) & 0x7F);
        irqrestore(irq);
        return 7;        
}

/* We always run in 24hr BCD mode without software DSE */
void ds12885_init(void)
{
    ds12885_write(0x0B, 0x06);
    ds12885_battery_good();
    ds12885_present = 1;
}

uint_fast8_t ds12885_interrupt(void)
{
    return ds12885_read(0x0C);
}

/* Set the DS12885 up as an interval timer at 64Hz */
void ds12885_set_interval(void)
{
    ds12885_write(0x0A, 0x2A);
    /* Periodic interrupt on */
    ds12885_write(0x0B, ds12885_read(0x0B)|0x40);
}

void ds12885_disable_interval(void)
{
    ds12885_write(0x0B, ds12885_read(0x0B)& ~0x40);
}
