#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <ds1302.h>
#include <devide.h>
#include <blkdev.h>
#include <ppide.h>
#include <rc2014.h>
#include "config.h"
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
	 * Pages 37 amd 38 are the second kernel bank
	 */
	for (i = 32 + 7; i < 64; i++)
		pagemap_add(i);

	/* finally add the common area */
	pagemap_add(32 + 3);

	ds1302_init();

	if (z180_present)
		kputs("Z180 CPU card detected.\n");
	if (acia_present)
		kputs("6850 ACIA detected at 0x80.\n");
	if (sio_present)
		kputs("Z80 SIO detected at 0x80.\n");
	if (sio1_present)
		kputs("Z80 SIO detected at 0x84.\n");
	if (ctc_present)
		kputs("Z80 CTC detected at 0x88.\n");
	if (ds1302_present)
		kputs("DS1302 detected at 0xC0.\n");
	/* Need to also look for 16550A at some point ? */
	/* Need to look for TMS9918/9918A */
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
	while (n)
		swapmap_init(n--);
#endif
}

void device_init(void)
{
#ifdef CONFIG_IDE
	devide_init();
#ifdef CONFIG_PPIDE
	ppide_init();
#endif
#endif
}
