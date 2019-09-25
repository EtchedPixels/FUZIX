#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <blkdev.h>
#include <devfdc765.h>

void pagemap_init(void)
{
	uint_fast8_t i;
 
	for (i = 0; i < 9; i++)
 		pagemap_add(36 + 3 * i);
}

/* Nothing to do for the map of init */
void map_init(void)
{
}

uint_fast8_t platform_param(char *p)
{
	used(p);
	return 0;
}


void device_init(void)
{
	ds1302_init();
	devide_init();
	rctty_init();
}
