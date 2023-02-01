#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <rtc.h>
#include <ds3234.h>
#ifdef CONFIG_RTC_DS3234

void ds3234_reg_write(uint_fast8_t r, uint_fast8_t val)
{
    ds3234_select(1);
    ds3234_write(r | 0x80);
    ds3234_write(val);
    ds3234_select(0);
}

uint_fast8_t ds3234_reg_read(uint_fast8_t r)
{
    ds3234_select(1);
    ds3234_write(r);
    r = ds3234_read();
    ds3234_select(0);
}

/*
 *	The other helpers only run at boot. This one runs live and because
 *	we mash on the serial line changes we need to keep interrupts off
 *	or we'll cause a storm of status change ints. Maybe later we can just
 *	temporarily turn off the lsr int, but that does risk losing clock
 *	ticks so might need plt_rtc_secs. For a clock who cares, but for SD
 *	it may need resolving in full.
 */
static void ds3234_read_clock(uint8_t *buf)
{
    uint_fast8_t n = 0;
    irqflags_t irq = di();
    ds3234_select(1);
    ds3234_write(0x00);
    while(n++ < 0x07)
        *buf++ = ds3234_read();
    ds3234_select(0);
    irqrestore(irq);
}

static void ds3234_write_clock(uint8_t *buf)
{
    uint_fast8_t n = 0;
    irqflags_t irq = di();
    ds3234_select(1);
    ds3234_write(0x80);
    while(n++ < 0x07)
        ds3234_write(*buf++);
    ds3234_select(0);
    irqrestore(irq);
}

uint_fast8_t plt_rtc_secs(void)
{
    /* We don't bind to the seconds as we don't need too and reading it
       is slow */
    return 0xFF;
}

/* Full RTC support (for read - no write yet) */
int plt_rtc_read(void)
{
	uint16_t len = sizeof(struct cmos_rtc);
	uint16_t y;
	struct cmos_rtc cmos;
	uint8_t *p = cmos.data.bytes;
	uint8_t rtc_buf[7];

	if (udata.u_count < len)
		len = udata.u_count;

	ds3234_read_clock(rtc_buf);

	y = rtc_buf[6];
	if (y > 0x70)
		y = 0x1900 | y;
	else
		y = 0x2000 | y;
	*p++ = y >> 8;
	*p++ = y;
	/* Shift the month by one BCD to be zero based. The returned
	   value from the RTC is always 1 or higher so no wrap */
	rtc_buf[5]--;
	*p++ = rtc_buf[5] & 0x1F;	/* Month */
	*p++ = rtc_buf[4];		/* Day of month */
	if ((rtc_buf[2] & 0x90) == 0x90) {	/* 12hr mode, PM */
		/* Add 12 BCD */
		rtc_buf[2] += 0x12;
		if ((rtc_buf[2] & 0x0F) > 9)	/* Overflow case */
			rtc_buf[2] += 0x06;
	}
	*p++ = rtc_buf[2];	/* Hour */
	*p++ = rtc_buf[1];	/* Minute */
	*p = rtc_buf[0];	/* Second */
	cmos.type = CMOS_RTC_BCD;
	if (uput(&cmos, udata.u_base, len) == -1)
		return -1;
	return len;
}

int plt_rtc_write(void)
{
	struct cmos_rtc cmos;
	uint8_t rtc_buf[7];
	if (udata.u_count != sizeof(struct cmos_rtc)) {
		udata.u_error = EINVAL;
		return -1;
	}
	if (uget(udata.u_base, &cmos, sizeof(struct cmos_rtc)) == -1)
		return -1;
	if (cmos.type != CMOS_RTC_BCD) {
		udata.u_error = EINVAL;
		return -1;
	}
	/* Time */
	rtc_buf[0] = cmos.data.bytes[6];
	rtc_buf[1] = cmos.data.bytes[5];
	rtc_buf[2] = cmos.data.bytes[4];
	/* Date */
	rtc_buf[3] = cmos.data.bytes[7] + 1;	/* 1 based */
	rtc_buf[4] = cmos.data.bytes[3];
	rtc_buf[5] = cmos.data.bytes[2];
	rtc_buf[6] = cmos.data.bytes[1];
	/* No upper century byte so we just assume 70+ is 19XX */
	ds3234_write_clock(rtc_buf);
}

#ifdef CONFIG_RTC_EXTENDED

uint_fast8_t rtc_nvread(uint_fast8_t r)
{
    uint_fast8_t v;
    irqflags_t irq = di();

    ds3234_reg_write(0x18, r);
    v = ds3234_reg_read(0x19);

    irqrestore(irq);
    return v;
}

void rtc_nvwrite(uint_fast8_t r, uint_fast8_t v)
{
    uint8_t n;
    irqflags_t irq = di();

    ds3234_reg_write(0x18, r);
    ds3234_reg_write(0x19, v);

    irqrestore(irq);
}

int plt_rtc_ioctl(uarg_t request, char *data)
{
    struct cmos_nvram *rtc = (struct cmos_nvram *)data;
    uint16_t r;
    int v;

    if (request == RTCIO_NVSIZE)
        return 256;

    r = ugetw(&rtc->offset);
    if (r > 255) {
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
    default:
        return -1;
    }
}

#endif
#endif
