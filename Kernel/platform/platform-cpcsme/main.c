#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <devinput.h>

uaddr_t ramtop = PROGTOP;

void plt_idle(void)
{
 __asm
  halt
 __endasm;
}

uint8_t timer_wait;

void plt_interrupt(void)
{
 tty_pollirq();
#if defined CONFIG_USIFAC_SERIAL
	tty_pollirq_usifac();
#endif
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
 *	common space into buffers.
 */
void plt_discard(void)
{
	uint16_t discard_size = PROGTOP - (uint16_t)bufpool_end;
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
