#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <blkdev.h>
#include <devide.h>
#include <propio2.h>

extern int strcmp(const char *, const char *);

void map_init(void)
{
}

/* Kernel in bank 0, user in banks 1-14, high 32K is bank 15 */
void pagemap_init(void)
{
	uint8_t i;
	for (i = 1; i < 15; i++)
		pagemap_add(i);
}

uint8_t platform_param(char *p)
{
	if (strcmp(p, "compumuse") == 0) {
		return 1;
	}
	return 0;
}

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
    swapmap_add(n--);
}

void device_init(void)
{
	devide_init();
	prop_sd_probe();
}
