#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <blkdev.h>
#include <devfdc765.h>

uaddr_t ramtop = PROGTOP;
uint16_t swap_dev = 0xFFFF;

extern uint16_t probe_bank(uint16_t);

void pagemap_init(void)
{
 uint8_t i;
 uint8_t m = 2;
 
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

/* On idle we spin checking for the terminals. Gives us more responsiveness
   for the polled ports */
void platform_idle(void)
{
  /* We don't want an idle poll and IRQ driven tty poll at the same moment */
  irqflags_t irq = di();
  tty_poll(); 
  irqrestore(irq);
}

void platform_interrupt(void)
{
 tty_poll();
 timer_interrupt();
 devfd_spindown();
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

