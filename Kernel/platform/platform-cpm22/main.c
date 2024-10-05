#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <tty.h>
#include <devtty.h>
#include <cpm.h>
#include <sysmod.h>

uaddr_t ramtop;
uint8_t cpm_busy = 0;
uint16_t sys_prog_top;
uint8_t plt_tick_present;
uint16_t swap_dev = 0xFFFF;
uint8_t copybanks;

struct sysinfo *info;

static uint8_t cpm_banks;

void pagemap_init(void)
{
	int i;

	/* Logical bank 0 is the kernel image */
 	for (i = 1; i < cpm_banks; i++)
 		pagemap_add(i);
}

void found_swap(uint16_t dev, uint16_t size)
{
	if (swap_dev != 0xFFFF) {
		kprintf("%dKb swap added.\n", size >> 1);
		swap_dev = dev;
		size /= SWAP_SIZE;
		while(--size)
			swapmap_add(size);
	}
}

void init_hardware_c(void)
{
	info = sysmod_info();
	sys_prog_top = info->common;
	sys_prog_top &= 0xFE00;		/* Align to 512 byte chunks */
	ramtop = PROGTOP;

	plt_tick_present = !!(info->features & FEATURE_TICK);
	
	cpm_banks = info->nbanks;
	copybanks = info->common >> 8;
	procmem = info->nbanks * (info->common >> 10);
	ramsize = procmem + 64;

	termios_mask[1] |= info->conflags;
	termios_mask[2] |= info->auxflags;

	kprintf("Common at %x, banks %d\n", info->common, info->nbanks);
}
		
/* Nothing to do for the map of init */
void map_init(void)
{
}

uint8_t plt_param(char *p)
{
	used(p);
	return 0;
}

void plt_idle(void)
{
	irqflags_t irq;
	sysmod_idle();
	irq = di();
	tty_pollirq();
	irqrestore(irq);
}

uint8_t plt_rtc_secs(void)
{
	return sysmod_rtc_secs();
}

struct blkbuf *bufpool_end = bufpool + NBUFS;

/*
 *	We pack discard into the memory image is if it were just normal
 *	code but place it at the end after the buffers. When we finish up
 *	booting we turn everything from the buffer pool to common into
 *	buffers.
 */
void plt_discard(void)
{
	uint16_t discard_size = info->common - (uint16_t)bufpool_end;
	bufptr bp = bufpool_end;

	discard_size /= sizeof(struct blkbuf);

	kprintf("%d buffers added\n", discard_size);

	bufpool_end += discard_size;

	memset( bp, 0, discard_size * sizeof(struct blkbuf) );

	for( bp = bufpool + NBUFS; bp < bufpool_end; ++bp ){
		bp->bf_dev = NO_DEVICE;
		bp->bf_busy = BF_FREE;
	}
}
