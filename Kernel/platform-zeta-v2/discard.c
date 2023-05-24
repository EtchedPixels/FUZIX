#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <ds1302.h>
#include "config.h"
#include "devrd.h"
#include <tinyide.h>

/* Everything in here is discarded after init starts */

void init_hardware_c(void)
{
    ramsize = 512;
    procmem = 512 - 64 - (DEV_RD_RAM_PAGES<<4);
    /* zero out the initial bufpool */
    memset(bufpool, 0, (char*)bufpool_end - (char*)bufpool);
}

void pagemap_init(void)
{
	int i;

	/* ZETA SBC V2 has RAM in the top 512 KiB of physical memory
	 * corresponding pages are 32-63 (page size is 16 KiB)
	 * Pages 32-34 are used by the kernel
	 * Page 35 is the common area
	 * Pages starting from DEV_RD_START are used by RAM disk
	 */
	for (i = 32 + 4; i < (DEV_RD_RAM_START >> 14); i++)
		pagemap_add(i);

	/* finally add the common area */
	pagemap_add(32 + 3);
}

void map_init(void)
{
}

void device_init(void)
{
	ds1302_init();
	uart0_init();
#ifdef CONFIG_TINYIDE_PPI
	ide_probe();
#endif
}

uint8_t plt_param(char *p)
{
    used(p);
    return 0;
}
