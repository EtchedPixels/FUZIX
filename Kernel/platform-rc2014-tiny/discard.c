#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <ds1302.h>
#include <rc2014.h>
#include <tinyide.h>
#include "config.h"

void map_init(void)
{
	ds1302_init();

	if (acia_present)
		kputs("6850 ACIA detected at 0x80.\n");
	if (sio_present)
		kputs("Z80 SIO detected at 0x80.\n");
	if (ctc_present) {
		plt_tick_present = 1;
		kputs("Z80 CTC detected at 0x88.\n");
	}
	if (ds1302_present)
		kputs("DS1302 detected at 0xC0.\n");
}

void device_init(void)
{
	ide_probe();
}
