#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <blkdev.h>
#include <devfdc765.h>

void pagemap_init(void)
{
	/* 0x03 is the kernel mapping */
	pagemap_add(0x83);
	pagemap_add(0x23);
	pagemap_add(0xA3);
	pagemap_add(0x0B);
	pagemap_add(0x8B);
	pagemap_add(0x2B);
	pagemap_add(0xAB);
	/* We don't use the other 8K blocks at this point. We should
	   eventually copy common into each bank and use part of that
	   top space for user and fast udata switching like Z180 */
}

/* Nothing to do for the map of init */
void map_init(void)
{
}

uint_fast8_t plt_param(char *p)
{
	return 0;
}


void device_init(void)
{
	ds1302_init();
	ppide_init();
	ide_probe();
	scsi_init();
	ch375_probe();
}
