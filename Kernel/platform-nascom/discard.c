#include <kernel.h>
#include <devtty.h>
#include <devscsi.h>
#include <devgm833.h>
#include <printf.h>
#include <blkdev.h>
#include <tty.h>
#include <nascom.h>

uint8_t io_page;
uint16_t bankmap;

void device_init(void)
{
#ifdef CONFIG_RTC
	/* Time of day clock */
	inittod();
#endif
	devscsi_init();
	/* Must come last as we want to allocate this for swap if no other
	   swap was found */
	gm833_init();
}

void map_init(void)
{
}

/* Add pagemap codes depending upon the present banks */
void pagemap_init(void)
{
	/* We didn't find any extra page mode memory */
	if (bankmap < (2 << 8))
		panic("no pagemode");
	/* Add the present banks (may be non contiguous). The
	   first bank is the kernel and not available */
	if (bankmap & 2)
		pagemap_add(0x22);
	if (bankmap & 4)
		pagemap_add(0x44);
	if (bankmap & 8)
		pagemap_add(0x88);
}

/* Swap partition on the hard disk */
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
	while (n)
		swapmap_add(n--);
}
