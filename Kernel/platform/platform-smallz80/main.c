#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <rtc.h>

uint16_t ramtop = PROGTOP;
uint16_t swap_dev = 0xFFFF;

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
 *	reti instructions) is dealt with for you.
 *
 *	Most platforms would read something to identify the interrupt source
 *	but in our case the only possible source is the RTC
 */
 
static uint8_t irqct;

__sfr __at 0x2D rtcD;
__sfr __at 0xF8 gpreg;

void plt_interrupt(void)
{
	uint8_t n, r;
	if (rtcD & 0x04) {
		irqct++;
		/* We get 64 pulses per second and must drop 4, in other words
		   drop one per 16 */
		if (irqct & 0xF0)
			timer_interrupt();
		rtcD &= ~0x04;

		/* Show the load on the lights */
		r = gpreg;
		r &= 0x8F;
		n = nready;
		if (n > 7)
			n = 7;
		n <<= 4;
		r |= n;
		gpreg = r;
		
	}
	tty_poll();
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
