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
}

void pagemap_init(void)
{
  /* The high bits do nothing but it's a cheap way to avoid 0x00 */
  pagemap_add(0x12);
  pagemap_add(0x10);

  ds1302_init();

  if (sio_present)
    kputs("Z80 SIO detected at 0x80.\n");
  if (sio1_present)
    kputs("Z80 SIO detected at 0x84.\n");
  if (ctc_present)
    kputs("Z80 CTC detected at 0x88.\n");
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
#ifdef CONFIG_IDE
	devide_init();
#endif
}
