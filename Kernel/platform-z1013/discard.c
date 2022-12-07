#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <devide.h>
#include <blkdev.h>
#include "config.h"

void map_init(void)
{
#ifdef CONFIG_RD_SWAP
	uint_fast8_t i;
	for (i = 0; i < MAX_SWAPS; i++)
		swapmap_init(i);
#endif
}

void device_init(void)
{
	rd_probe();
#ifdef CONFIG_IDE
	devide_init();
#endif
}
