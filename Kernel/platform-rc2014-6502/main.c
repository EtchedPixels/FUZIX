#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <blkdev.h>
#include <devide.h>
#include <devtty.h>

uint8_t kernel_flag = 1;
uint16_t swap_dev = 0xFFFF;

void platform_idle(void)
{
    irqflags_t flags = di();
    tty_poll();
    irqrestore(flags);
}

void do_beep(void)
{
}

/*
 * Map handling: We have flexible paging. Each map table consists of a set of pages
 * with the last page repeated to fill any holes.
 */

void pagemap_init(void)
{
    int i;
    /* 32-35 are the kernel, 36-39 / 40-43 / etc are user */
    /* Add 36-39 last as we hardcode 36 into our init creation in tricks.s */
    for (i = 7; i >= 1; i--)
        pagemap_add(32 + i * 4);
}

void map_init(void)
{
}

uint8_t platform_param(char *p)
{
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
#ifdef SWAPDEV
	n = blk->lba_count[m - 1] / SWAP_SIZE;
	if (n > MAX_SWAPS)
		n = MAX_SWAPS;
	while (n)
		swapmap_init(n--);
#endif
}

void device_init(void)
{
#ifdef CONFIG_IDE
	devide_init();
#endif
}

/* For now this and the supporting logic don't handle swap */

extern uint8_t hd_map;
extern void hd_read_data(uint8_t *p);
extern void hd_write_data(uint8_t *p);

void devide_read_data(void)
{
	if (blk_op.is_user)
		hd_map = 1;
	else
		hd_map = 0;
	hd_read_data(blk_op.addr);	
}

void devide_write_data(void)
{
	if (blk_op.is_user)
		hd_map = 1;
	else
		hd_map = 0;
	hd_write_data(blk_op.addr);	
}

