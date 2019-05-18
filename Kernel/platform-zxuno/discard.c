#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <blkdev.h>
#include <zxuno.h>

extern uint8_t fuller, kempston, kmouse, kempston_mbmask;


void pagemap_init(void)
{
	if (probe_zxuno())
		configure_zxuno();
//	else
//		panic("not zxuno");
	/* These are always present */
	kempston = 1;
	kmouse = 1;
	fuller = 1;
	/* Check what extensions we can assume FIXME */
}

/* string.c
 * Copyright (C) 1995,1996 Robert de Bath <rdebath@cix.compulink.co.uk>
 * This file is part of the Linux-8086 C library and is distributed
 * under the GNU Library General Public License.
 */
static int strcmp(const char *d, const char *s)
{
	register char *s1 = (char *) d, *s2 = (char *) s, c1, c2;

	while ((c1 = *s1++) == (c2 = *s2++) && c1);
	return c1 - c2;
}

uint8_t platform_param(char *p)
{
	return 0;
}

/* Nothing to do for the map of init */
void map_init(void)
{
}

void platform_copyright(void)
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
