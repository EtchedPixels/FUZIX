#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <rtc.h>
#include <bios.h>

/* Full RTC support (for read - no write yet) */
int plt_rtc_read(void)
{
        int err;
	uint16_t len = sizeof(struct cmos_rtc);
	struct cmos_rtc cmos;

	if (udata.u_count < len)
		len = udata.u_count;

        if ((err = fuzixbios_rtc_get(&cmos)) < 0) {
                udata.u_error = err;
                return -1;
        }
	if (uput(&cmos, udata.u_base, len) == -1)
		return -1;
	return len;
}

int plt_rtc_write(void)
{
	uint16_t len = sizeof(struct cmos_rtc);
	struct cmos_rtc cmos;

	if (udata.u_count != len) {
	        udata.u_error = EINVAL;
	        return -1;
        }
	if (uget(&cmos, udata.u_base, len) == -1)
		return -1;
        return fuzixbios_rtc_set(&cmos);
}
