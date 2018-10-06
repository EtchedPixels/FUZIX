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
#ifdef CONFIG_VFD_TERM
    vfd_term_init();
#endif
    ramsize = 512;
    procmem = 512 - 80;
}

void pagemap_init(void)
{
	int i;

	/* RC2014 512/512K has RAM in the top 512 KiB of physical memory
	 * corresponding pages are 32-63 (page size is 16 KiB)
	 * Pages 32-34 are used by the kernel
	 * Page 35 is the common area for init
         * Page 36 is the disk cache
	 */
	for (i = 32 + 5; i < 64; i++)
		pagemap_add(i);

	/* finally add the common area */
	pagemap_add(32 + 3);
}

void map_init(void)
{
	/* Point the buffers into the 16-32K range that will be used by
	   the buffer remap. It's an ideal location because it has no
	   vectors and is low enough it will overlay only code so we never
	   worry about copying into the overlaid address space */
	bufptr bp = bufpool;
	uint8_t *p = (uint8_t *)0x4000;
	while(bp < bufpool_end) {
		bp++->__bf_data = p;
		p += BLKSIZE;
	}
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
