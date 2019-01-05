#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <ds1302.h>
#include <devide.h>
#include <blkdev.h>
#include <scm_monitor.h>
#include "config.h"

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
	while(n)
		swapmap_init(n--);
}

void system_init(void)
{
	scm_version();
	switch(scm_hw_info >> 8) {
	case SCM_SC114:
		kputs("Small Computer 114.\n");
		/* RAM switch port 30 bit 0, ROM port 38 bit 0 */
		/* Second ACIA or an SIO/2 means two serial */
		if (scm_hw_info & 6)
			numtty = 2;
		break;
	case SCM_SC108:
		kputs("Small Computer 108.\n");
		/* RAM switch port 38 bit 7, ROM port 38 bit 0 */
		/* Want to merge in this port */
	/* And the SCM SC101 uses outs to port 18 to select ram bank 0 ,
	   19 1, and 1c/1d for ROM in and out. However there's only one of it
	   so probably not worth the trouble ! */
	default:
		/* Unsupported */
		panic("Unsupported platform.\n");
	}
}

void device_init(void)
{
	system_init();
	ds1302_init();
#ifdef CONFIG_IDE
	devide_init();
#endif
}
