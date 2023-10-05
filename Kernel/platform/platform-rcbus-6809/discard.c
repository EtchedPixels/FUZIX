#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <blkdev.h>

/*
 * We have flexible 16K paging
 */

void pagemap_init(void)
{
	uint8_t i;
	for (i = 0x24; i <= 0x3F; i++)
		pagemap_add(i);
	/* Common shared for init */
	pagemap_add(0x23);
}

uint8_t plt_param(char *p)
{
	return 0;
}

void map_init(void)
{
}
