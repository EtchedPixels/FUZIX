#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <ds1302.h>
#include <tinyide.h>
#include <rcbus.h>
#include "config.h"

void map_init(void)
{
	if (*((uint8_t *)0xFFFF) & 1)
		panic("R16 error fix/modified board required.\n");
	ds1302_init();
	if (ds1302_present)
		kputs("DS1302 detected at 0xC0.\n");
	/* Default clashes with the CF adapter for Simple 80 */
	if (ctc_present) {
		plt_tick_present = 1;
		kputs("Z80 CTC detected at 0xD0.\n");
	}
}

void device_init(void)
{
	ide_probe();
}
