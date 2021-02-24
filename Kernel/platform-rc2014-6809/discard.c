#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <blkdev.h>

/*
 * We have flexible 16K paging but to get started we are just banking the low
 * 48K
 */

void pagemap_init(void)
{
	uint8_t i;
	/* 32,33,34,35 kernel
	   36,37,38,35 user 0
	   39,49,41,35  user 1 etc */
	for (i = 1; i <= 9; i++)
		pagemap_add(33 + 3 * i);
}

uint8_t platform_param(char *p)
{
	return 0;
}

void map_init(void)
{
}
