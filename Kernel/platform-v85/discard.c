#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <blkdev.h>
#include <devfdc765.h>

void pagemap_init(void)
{
 uint_fast8_t i;
 uint_fast8_t m = 2;
 
 for (i = 1; i < 8; i++) {
  if (probe_bank(m) == 0) {
	pagemap_add(m);
	ramsize += 48;
	procmem += 48;
  }
  m <<= 1;
 }
 if (procmem < 96)
 	panic("insufficient memory");
}

/* Nothing to do for the map of init */
void map_init(void)
{
}

uint8_t platform_param(char *p)
{
 used(p);
 return 0;
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
	
	if (blk->lba_count[m - 1] > 0xFFFF)
		n = 0xFFFF;
	else
		n = (uint16_t)blk->lba_count[m-1];
	n /= SWAP_SIZE;
	if (n > MAX_SWAPS)
		n = MAX_SWAPS;
#ifdef SWAPDEV
	while (n)
		swapmap_init(n--);
#endif
}

