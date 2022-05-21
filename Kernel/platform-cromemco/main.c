#include <kernel.h>
#include <kdata.h>
#include <devtty.h>
#include <printf.h>
#include <irq.h>

uaddr_t ramtop = PROGTOP;

void plt_idle(void)
{
 __asm
  halt
 __endasm;
}

__sfr __at 0x08 timer4;

void plt_interrupt(void)
{
 timer4 = 156;
 tty_drain();
 timer_interrupt();
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
