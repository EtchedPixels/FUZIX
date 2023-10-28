#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <devrtc.h>
#include <rtc.h>

#define clkc	25
#define clkd	26

void zrtc_init(void)
{
	out(clkc, 0xFF);
	out(clkc, 0x00);
	inittod();
}

uint_fast8_t plt_rtc_secs(void)
{
	return in(clkd);
}

int plt_rtc_read(void)
{
	struct cmos_rtc cmos;
	register uint8_t *p = cmos.data.bytes;
	uint16_t year;
	irqflags_t irqflags;
	uint16_t len = sizeof(struct cmos_rtc);

	if (udata.u_count < len)
		len = udata.u_count;

	/* Don't interfere with the rtc_secs fast path */
	irqflags = di();

	out(clkc, 7);
	year = in(clkd);
	year += 1900;
	*p++ = year & 0xFF;
	*p++ = year >> 8;
	out(clkc, 6);
	*p++ = in(clkd);	/* month 0-11 */
	out(clkc,5 );
	*p++ = in(clkd);	/* day of month 1-31 */
	out(clkc, 2);
	*p++ = in(clkd);	/* Hour 0-23 */
	out(clkc, 1);
	*p++ = in(clkd);	/* Minutes 0-59 */
	out(clkc, 0);
	*p = in(clkd);		/* Seconds 0-59, leave clkc as needed for rtc_secs */
	irqrestore(irqflags);
	cmos.type = CMOS_RTC_DEC;
	if (uput(&cmos, udata.u_base, len) == -1)
		return -1;
	return len;
}

int plt_rtc_write(void)
{
	udata.u_error = EOPNOTSUPP;
	return -1;
}
