#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <blkdev.h>

uint8_t plt_param(char *p)
{
	return 0;
}

/* Nothing to do for the map of init */
void map_init(void)
{
#if defined EXTENDED_RAM_512 || defined EXTENDED_RAM_1024	
	uint_fast8_t i;
	for (i = 0; i < MAX_SWAPS; i++)
		swapmap_init(i);
#endif
}

void plt_copyright(void)
{
	kprintf("Amstrad CPC6128 platform\nCopyright (c) 2024-2025 Antonio J. Casado Alias\n");
}
/*
void ide_reset(void)
{
	ide_std_reset();
}
*/

