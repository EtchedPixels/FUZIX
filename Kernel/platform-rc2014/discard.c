#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <ds1302.h>
#include <devide.h>
#include <blkdev.h>
#include "config.h"
#include "devrd.h"
#include "vfd-term.h"

/* Everything in here is discarded after init starts */

void init_hardware_c(void)
{
//    vfd_debug_init();
#ifdef CONFIG_VFD_TERM
    vfd_term_init();
#endif
    ramsize = 512;
    procmem = 512 - 64 - (DEV_RD_RAM_PAGES<<4);
    /* zero out the initial bufpool */
    memset(bufpool, 0, (char*)bufpool_end - (char*)bufpool);
}

void pagemap_init(void)
{
	int i;

	/* RC2014 512/512K has RAM in the top 512 KiB of physical memory
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

/*
 *	This function is called for partitioned devices if a partition is found
 *	and marked as swap type. The first one found will be used as swap. We
 *	only support one swap device.
 */
void platform_swap_found(uint8_t letter, uint8_t m)
{
  blkdev_t *blk = blk_op.blkdev;
  uint16_t n;
  if (swap_dev != 0xFFFF)
    return;
  letter -= 'a';
  kputs("(swap) ");
  swap_dev = letter << 4 | m;
  n = blk->lba_count[m - 1] / SWAP_SIZE;
  if (n > MAX_SWAPS)
    n = MAX_SWAPS;
#ifdef SWAPDEV
  while(n)
    swapmap_add(n--);
#endif
}

void device_init(void)
{
	ds1302_init();
#ifdef CONFIG_IDE
	devide_init();
#endif
}
