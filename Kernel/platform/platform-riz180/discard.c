#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <tinyide.h>
#include <tinysd.h>
#include "config.h"
#include <z180.h>
#include <ds1302.h>
#include <netdev.h>
#include "riz180.h"

void init_hardware_c(void)
{
	ramsize = 128;
	procmem = 112;
	/* zero out the initial bufpool */
	memset(bufpool, 0, (char *) bufpool_end - (char *) bufpool);
}

void pagemap_init(void)
{
	ds1302_init();
	if (ds1302_present)
		kputs("DS1302 detected at 0x0C.\n");
	pagemap_add(0x52);
	pagemap_add(0x44);
}

void map_init(void)
{
}

uint8_t plt_param(char *p)
{
	used(p);
	return 0;
}

void device_init(void)
{
	ide_probe();
#ifdef CONFIG_TD_SD
	sd_probe();
#endif
#ifdef CONFIG_NET
	netdev_init();
#endif
}
