#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <devrtc.h>
#include <rtc.h>

__sfr __at 25 clkc;
__sfr __at 26 clkd;


void zrtc_init(void)
{
	clkc = 0xFF;
	clkc = 0x00;
	inittod();
}

uint8_t platform_rtc_secs(void)
{
	return clkd;
}

int platform_rtc_read(void)
{
	struct cmos_rtc cmos;
	uint8_t *p = cmos.data.bytes;
	uint16_t year;
	irqflags_t irqflags;
	uint16_t len = sizeof(struct cmos_rtc);

	if (udata.u_count < len)
		len = udata.u_count;

	/* Don't interfere with the rtc_secs fast path */
	irqflags = di();

	clkc = 7;
	year = clkd;
	year += 1900;
	*p++ = year & 0xFF;
	*p++ = year >> 8;
	clkc = 6;
	*p++ = clkd;	/* month 0-11 */
	clkc = 5;
	*p++ = clkd;	/* day of month 1-31 */
	clkc = 2;
	*p++ = clkd;	/* Hour 0-23 */
	clkc = 1;
	*p++ = clkd;	/* Minutes 0-59 */
	clkc = 0;
	*p = clkc;	/* Seconds 0-59, leave clkc as needed for rtc_secs */
	irqrestore(irqflags);
	cmos.type = CMOS_RTC_DEC;
	if (uput(&cmos, udata.u_base, len) == -1)
		return -1;
	return len;
}

int platform_rtc_write(void)
{
	udata.u_error = EOPNOTSUPP;
	return -1;
}
