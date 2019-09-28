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
#ifdef OLD
	for (i = 0; i < 9; i++)
 		pagemap_add(36 + 3 * i);
#else
	/* 0x03 is the kernel mapping */
	pagemap_add(0x83);
	pagemap_add(0x43);
	pagemap_add(0xC3);
	pagemap_add(0x0B);
	pagemap_add(0x8B);
	pagemap_add(0x4B);
	pagemap_add(0xCB);
	/* We don't use the other 8K blocks at this point. We should
	   eventually copy common into each bank and use part of that
	   top space for user and fast udata switching like Z180 */
#endif
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
}
