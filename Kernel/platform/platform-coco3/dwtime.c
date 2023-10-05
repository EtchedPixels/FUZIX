/* Drivewire Time Keeping */

#include <kernel.h>
#include <drivewire.h>
#include <printf.h>
#include <kdata.h>
#include "config.h"


/* Called every every decisec from timer.c */
uint8_t plt_rtc_secs(void)
{
	uint8_t t[6];
	/* poll dw time and return it */
	dw_rtc_read(t);
	return t[5];
}
