#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <ds1302.h>
#include <devide.h>
#include <blkdev.h>
#include <rc2014.h>
#include "config.h"

void map_init(void)
{
	ds1302_init();
	if (ds1302_present)
		kputs("DS1302 detected at 0xC0.\n");
	/* Default clashes with the CF adapter for Simple 80 */
	if (ctc_present)
		kputs("Z80 CTC detected at 0xD0.\n");
}

void device_init(void)
{
	devide_init();
}
