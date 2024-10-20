#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <kmod.h>
#include <printf.h>
#include <devtty.h>
#include <devinput.h>
//#include <net_w5x00.h>

uint16_t ramtop = PROGTOP;
uint16_t swap_dev = 0xFFFF;

/* On idle we spin checking for the terminals. Gives us more responsiveness
   for the polled ports */
void plt_idle(void)
{
  /* We don't want an idle poll and IRQ driven tty poll at the same moment */
  __asm
   halt
  __endasm;
}

uint8_t timer_wait;

void plt_interrupt(void)
{
 tty_pollirq();
 timer_interrupt();
 poll_input();
 if (timer_wait)
  wakeup(&timer_interrupt);
 devfd_spindown();
}

/*
 *	So that we don't suck in a library routine we can't use from
 *	the runtime
 */

size_t strlen(const char *p)
{
  size_t len = 0;
  while(*p++)
    len++;
  return len;
}

/* This points to the last buffer in the disk buffers. There must be at least
   four buffers to avoid deadlocks. */
struct blkbuf *bufpool_end = bufpool + NBUFS;

/*
 *	We pack discard into the memory image is if it were just normal
 *	code but place it at the end after the buffers. When we finish up
 *	booting we turn everything from the buffer pool to the start of
 *	user space into buffers.
 *
 *	We don't touch discard. Discard is just turned into user space.
 */
void plt_discard(void)
{
	uint16_t discard_size = ((uint16_t)&udata) - (uint16_t)bufpool_end;
	bufptr bp = bufpool_end;
#ifdef CONFIG_KMOD
	kmod_init(bufpool_end, &udata);
#endif

	discard_size /= sizeof(struct blkbuf);

	kprintf("%d buffers added\n", discard_size);

	bufpool_end += discard_size;

	memset( bp, 0, discard_size * sizeof(struct blkbuf) );

	for( bp = bufpool + NBUFS; bp < bufpool_end; ++bp ){
		bp->bf_dev = NO_DEVICE;
		bp->bf_busy = BF_FREE;
	}
}

unsigned plt_kmod_set(uint8_t *top)
{
	/* Make sure all disk buffers are on disk */
	sync();
	/* Wind back until bufpool end is below the modules */
	while(bufpool_end > (void *)top)
		bufpool_end--;
	/* Any buffers lost we already wrote to disk, any new lookups will
	   not find them, and we know there are no other outstanding references
	   - sometimes having a dumb I/O layer is a win */
	return 0;
}

#ifndef SWAPDEV
/* Adding dummy swapper since it is referenced by tricks.s */
void swapper(ptptr p)
{
  p;
}
#endif

#ifdef CONFIG_NET_W5100

uint8_t w5x00_readcb(uint16_t off)
{
}

uint8_t w5x00_readsb(uint8_t s, uint16_t off)
{
}

uint16_t w5x00_readcw(uint16_t off)
{
}

uint16_t w5x00_readsw(uint8_t s, uint16_t off)
{
}

void w5x00_bread(uint16_t bank, uint16_t off, void *pv, uint16_t n)
{
}

void w5x00_breadu(uint16_t bank, uint16_t off, void *pv, uint16_t n)
{
}

void w5x00_writecb(uint16_t off, uint8_t n)
{
}

void w5x00_writesb(uint8_t sock, uint16_t off, uint8_t n)
{
}

void w5x00_writecw(uint16_t off, uint16_t n)
{
}

void w5x00_writesw(uint8_t sock, uint16_t off, uint16_t n)
{
}

void w5x00_bwrite(uint16_t bank, uint16_t off, void *pv, uint16_t n)
{
}

void w5x00_bwriteu(uint16_t bank, uint16_t off, void *pv, uint16_t n)
{
}

void w5x00_setup(void)
{
}
#endif
