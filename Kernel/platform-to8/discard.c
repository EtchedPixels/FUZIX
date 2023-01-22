#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <device.h>
#include <devtty.h>
#include <blkdev.h>

void map_init(void)
{
}

void pagemap_init(void)
{
	/* Assumes 512K for the moment */
	uint8_t i;
	uint8_t pmax = ramsize >> 4;
	for (i = 0x06; i < pmax ; i+= 0x02)
		pagemap_add(i);
}
