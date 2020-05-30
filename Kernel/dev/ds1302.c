/*-----------------------------------------------------------------------*/
/* DS1202 and DS1302 Serial Real Time Clock driver                       */
/* 2014-12-30 Will Sowerbutts                                            */
/*-----------------------------------------------------------------------*/

#define _DS1302_PRIVATE

#include <kernel.h>
#include <kdata.h>
#include <stdbool.h>
#include <printf.h>
#include <rtc.h>
#include <ds1302.h>

uint8_t ds1302_present;
uint8_t rtc_defer;

void ds1302_send_byte(uint_fast8_t byte)
{
    uint8_t i;

#ifdef DS1302_DEBUG
    kprintf("ds1302: send byte 0x%x\n", byte);
#endif
    /* drive the data pin */
    ds1302_set_pin_data_driven(true);

    /* clock out one byte, LSB first */
    for(i=0; i<8; i++){
        ds1302_set_pin_clk(false);
        /* for data input to the chip the data must be valid on the rising edge of the clock */
        ds1302_set_pin_data(byte & 1);
        byte >>= 1;
        ds1302_set_pin_clk(true);
    }
}

uint_fast8_t ds1302_receive_byte(void)
{
    uint8_t i, b;

    /* tri-state the data pin */
    ds1302_set_pin_data_driven(false);

    /* clock in one byte, LSB first */
    b = 0;
    for(i=0; i<8; i++){
        ds1302_set_pin_clk(false);
        b >>= 1;
        /* data output from the chip is presented on the falling edge of each clock */
        /* note that output pin goes high-impedance on the rising edge of each clock */
        if(ds1302_get_pin_data())
            b |= 0x80;
        ds1302_set_pin_clk(true);
    }

    return b;
}

void ds1302_write_register(uint_fast8_t reg, uint_fast8_t val)
{
    ds1302_set_pin_ce(true);
    ds1302_send_byte(reg);
    ds1302_send_byte(val);
    ds1302_set_pin_ce(false);
    ds1302_set_pin_clk(false);
}
uint_fast8_t uint8_from_bcd(uint_fast8_t value)
{
    return (value & 0x0F) + (10 * (value >> 4));
}

void ds1302_read_clock(uint8_t *buffer, uint_fast8_t length)
{
    uint8_t i;
    irqflags_t irq = di();

    platform_ds1302_setup();

    ds1302_set_pin_ce(true);
    ds1302_send_byte(0x81 | 0x3E); /* burst read all calendar data */
    for(i=0; i<length; i++){
        buffer[i] = ds1302_receive_byte();
#ifdef DS1302_DEBUG
        kprintf("ds1302: received byte 0x%x index %d\n", buffer[i], i);
#endif
    }
    ds1302_set_pin_clk(false);
    ds1302_set_pin_ce(false);

    platform_ds1302_restore();

    irqrestore(irq);
}

#ifdef CONFIG_RTC_EXTENDED

static uint8_t ds1302_read_register(uint_fast8_t reg)
{
    uint8_t val;
    ds1302_set_pin_ce(true);
    ds1302_send_byte(reg);
    val = ds1302_receive_byte();
    ds1302_set_pin_ce(false);
    ds1302_set_pin_clk(false);
    return val;
}

uint_fast8_t rtc_nvread(uint_fast8_t r)
{
    uint_fast8_t v;
    irqflags_t irq;

    irq = di();

    platform_ds1302_setup();

    v = ds1302_read_register(0xC1 + 2 * r);

    platform_ds1302_restore();

    irqrestore(irq);
    return v;
}

void rtc_nvwrite(uint_fast8_t r, uint_fast8_t v)
{
    irqflags_t irq;
    uint8_t n;

    irq = di();

    platform_ds1302_setup();
    
    n = ds1302_read_register(0x8F);

    /* Turn off write protect if we need to */
    if (n & 0x80)
        ds1302_write_register(0x8E, n & 0x7F);

    ds1302_write_register(0xC0 + 2 * r, v);

    /* Restore write protect if it was set */
    if (n & 0x80)
        ds1302_write_register(0x8E, n);

    platform_ds1302_restore();

    irqrestore(irq);
}

int platform_rtc_ioctl(uarg_t request, char *data)
{
    struct cmos_nvram *rtc = (struct cmos_nvram *)data;
    uint16_t r;
    int v;

    if (request == RTCIO_NVSIZE)
        return 31;

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
    default:
        return -1;
    }
}

#endif

/* define CONFIG_RTC in platform's config.h to hook this into timer.c */
uint_fast8_t platform_rtc_secs(void)
{
    uint8_t buffer;
    /* On some platforms the RTC is accessed via a shared interface, so
       we skip seconds polling off interrupts if directed to do so */
    if (ds1302_present && !rtc_defer) {
        ds1302_read_clock(&buffer, 1);   /* read out only the seconds value */
        return uint8_from_bcd(buffer & 0x7F); /* mask off top bit (clock-halt) */
    }
    return 0xFF;
}

static uint8_t rtc_buf[8];

/* Full RTC support (for read - no write yet) */
int platform_rtc_read(void)
{
	uint16_t len = sizeof(struct cmos_rtc);
	uint16_t y;
	struct cmos_rtc cmos;
	uint8_t *p = cmos.data.bytes;

	if (!ds1302_present) {
		udata.u_error = EOPNOTSUPP;
		return -1;
	}

	if (udata.u_count < len)
		len = udata.u_count;

	ds1302_read_clock(rtc_buf, 7);

	y = rtc_buf[6];
	if (y > 0x70)
		y = 0x1900 | y;
	else
		y = 0x2000 | y;
	*p++ = y >> 8;
	*p++ = y;
	rtc_buf[4]--;		/* 0 based */
	if ((rtc_buf[4] & 0x0F) > 9)	/* Overflow case */
		rtc_buf[4] -= 0x06;
	*p++ = rtc_buf[4];	/* Month */
	*p++ = rtc_buf[3];	/* Day of month */
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

int platform_rtc_write(void)
{
	udata.u_error = EOPNOTSUPP;
	return -1;
}
