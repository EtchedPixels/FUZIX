#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <device.h>
#include <devtty.h>
#include <blkdev.h>

void map_init(void)
{
	uint8_t i;
	/* We put swap on the start of slice 0, but with the first 64K free
	   so we can keep the OS image linearly there */
	for (i = 0; i < MAX_SWAPS; i++)
		swapmap_init(i + 128);
}

void pagemap_init(void)
{
	/* Assumes 512K for the moment */
	uint8_t i;
	uint8_t pmax = ramsize >> 4;
	for (i = 0x06; i < pmax ; i+= 0x02)
		pagemap_add(i);
}
