#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <rtc.h>

uint16_t ramtop = PROGTOP;

/*
 *	This routine is called continually when the machine has nothing else
 *	it needs to execute. On a machine with entirely interrupt driven
 *	hardware this could just halt for interrupt.
 *
 *	There are no serial interrupts on this system normally.
 */
void plt_idle(void)
{
	/* Disable interrupts so we don't accidentally process a polled tty
	   and interrupt call at once and make a mess */
	irqflags_t irq = di();
	tty_poll();
	/* Restore prior state. */
	irqrestore(irq);
}

/*
 *	This routine is called from the interrupt handler code to process
 *	interrupts. All of the nasty stuff (register saving, bank switching,
 *	reti instructions) is dealt with for you. We have a very fast interrupt
 *	so we downcount it in asm before coming here.
 */
 
extern uint8_t irq_cause;

void plt_interrupt(void)
{
	if (irq_cause & 2)
		tty_poll();
	if (irq_cause & 1)
		timer_interrupt();
	/* TODO bit banger port */
	irq_cause = 0;
}

/* This points to the last buffer in the disk buffers. There must be at least
   four buffers to avoid deadlocks. */
struct blkbuf *bufpool_end = bufpool + NBUFS;

/*
 *	We pack discard into the memory image is if it were just normal
 *	code but place it at the end after the buffers. When we finish up
 *	booting we turn everything from the buffer pool to common into
 *	buffers. This blows away the _DISCARD segment.
 */
void plt_discard(void)
{
	uint16_t discard_size = (uint16_t)udata - (uint16_t)bufpool_end;
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
