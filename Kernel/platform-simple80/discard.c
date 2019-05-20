#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <ds1302.h>
#include <devide.h>
#include <blkdev.h>
#include <rc2014.h>
#include "config.h"

void map_init(void)
{
        /* Default clashes with the CF adapter for Simple 80 */
	if (ctc_present)
		kputs("Z80 CTC detected at 0xA0.\n");
	ds1302_init();
	if (ds1302_present)
		kputs("DS1302 detected at 0xC0.\n");
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
  while(n)
    swapmap_init(n--);
}

void device_init(void)
{
	devide_init();
}
