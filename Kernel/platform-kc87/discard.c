#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <tinyide.h>
#include "config.h"

void map_init(void)
{
#ifdef CONFIG_RD_SWAP
	uint_fast8_t i;
	for (i = 0; i < MAX_SWAPS; i++)
		swapmap_init(i);
#endif
	/* Update according to build size */
	procmem = PROC_SIZE;
}

void device_init(void)
{
	rd_probe();
	ide_probe();
}
