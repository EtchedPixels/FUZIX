/*
 *	PC style 12885/12887 and similar real time clock/NVRAM
 *
 *	One weirdness here is that the century feature only works in BCD mode
 */

#include <kernel.h>
#include <kdata.h>
#include <rtc.h>
#include <bq4845.h>
#include <printf.h>

#ifdef CONFIG_RTC_BQ4845

uint_fast8_t bq4845_present;


/* define CONFIG_RTC in platform's config.h to hook this into timer.c */

uint_fast8_t plt_rtc_secs(void)
{
    uint_fast8_t v = bq4845_read(BQ4845_SEC);
    /* Turn it native */
    return (v & 0x0F) + (((v & 0xF0) >> 4) * 10);
}

int bq4845_battery_good(void)
{
    uint_fast8_t r= bq4845_read(BQ4845_REGD);
    if (r & REGD_BVF)
        return 1;
    kputs("bq4845: battery low\n");
    return 0;
}

int plt_rtc_read(void)
{
	uint16_t len = sizeof(struct cmos_rtc);
	struct cmos_rtc cmos;
	uint8_t *p = cmos.data.bytes;
	irqflags_t irq;

	if (!bq4845_present) {
		udata.u_error = EOPNOTSUPP;
		return -1;
	}

        bq4845_battery_good();
	if (udata.u_count < len)
		len = udata.u_count;


        irq = di();
        bq4845_write(BQ4845_REGE, REGE_DSE|REGE_2412|REGE_STOP|REGE_UTI);
	p[1] = bq4845_read(BQ4845_YR);
	if (p[1] > 70)
	    p[0] = 0x19;
        else
            p[0] = 0x20;
        p += 2;
	*p++ = bq4845_read(BQ4845_MON) - 1;   /* convert to 0-based month */
	*p++ = bq4845_read(BQ4845_DOM);
	*p++ = bq4845_read(BQ4845_HR);
	*p++ = bq4845_read(BQ4845_MIN);
	*p = bq4845_read(BQ4845_SEC);
        bq4845_write(BQ4845_REGE, REGE_DSE|REGE_2412|REGE_STOP);
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

	if (!bq4845_present) {
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

        bq4845_battery_good();
        irq = di();
        /* Turn the SET bit on */
        p++; /* No century byte */
        bq4845_write(BQ4845_REGE, REGE_DSE|REGE_2412|REGE_STOP|REGE_UTI);
        bq4845_write(BQ4845_YR,  *p++);
	bq4845_write(BQ4845_MON, *p++ + 1);  /* convert from 0-based month */
        bq4845_write(BQ4845_DOM, *p++);
        bq4845_write(BQ4845_HR,  *p++);
        bq4845_write(BQ4845_MIN, *p++);
        bq4845_write(BQ4845_SEC, *p);
        bq4845_write(BQ4845_REGE, REGE_DSE|REGE_2412|REGE_STOP);
        irqrestore(irq);
        return len;        
}

void bq4845_init(void)
{
        /* 24 hour mode, DST */
        bq4845_write(BQ4845_REGE, REGE_DSE|REGE_2412|REGE_STOP|REGE_UTI);
        /* Turn off any timer interrupt stuff */
        bq4845_write(BQ4845_REGB, 0);
        bq4845_write(BQ4845_REGC, 0);
        bq4845_write(BQ4845_REGE, REGE_DSE|REGE_2412|REGE_STOP);
	bq4845_battery_good();
	bq4845_present = 1;
}

#endif
