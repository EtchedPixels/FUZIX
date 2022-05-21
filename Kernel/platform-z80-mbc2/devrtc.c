#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <rtc.h>
#include <mbc2.h>

/* Full RTC support (for read - no write yet) */
int plt_rtc_read(void)
{
        irqflags_t irq;
	uint16_t len = sizeof(struct cmos_rtc);
	uint16_t y;
	struct cmos_rtc cmos;
	uint8_t *p = cmos.data.bytes;

	if (udata.u_count < len)
		len = udata.u_count;

        irq = di();

        opcode = OP_GET_RTC;
        p[7] = 0;
        p[6] = opread;		/* Seconds 0-59 */
        p[5] = opread;		/* Minutes 0-59 */
        p[4] = opread;		/* Hours 0-23 */
        p[3] = opread;		/* Day of month 1-31 */
        p[2] = opread;		/* Month 1-12 */
        y = opread + 2000;	/* Year 2000-.. */

        irqrestore(irq);

        p[0] = y;
        p[1] = y >> 8;
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
