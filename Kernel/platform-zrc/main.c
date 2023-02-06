#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <rtc.h>
#include <ds1302.h>
#include <net_w5x00.h>

uint16_t ramtop = PROGTOP;
uint16_t swap_dev = 0xFFFF;
uint8_t timermsr = 0;

uint8_t plt_tick_present;

/* Real time clock support for DS1302 driver - port and other pins */
uint16_t rtc_port = 0x0C;
uint8_t rtc_shadow;

/*
 *	This routine is called continually when the machine has nothing else
 *	it needs to execute. On a machine with entirely interrupt driven
 *	hardware this could just halt for interrupt.
 *
 *	The SBCv2 has no interrupts so we must call sync_clock(), and as the
 *	PropIO tty is not interrupt driven we also poll the ttys. This gives
 *	us a nice interactive feel when the machine is idle, even if a polled
 *	tty can otherwise suck.
 */
void plt_idle(void)
{
	/* Disable interrupts so we don't accidentally process a polled tty
	   and interrupt call at once and make a mess */
	irqflags_t irq = di();
	sync_clock();
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
 *	but in our case the only possible source is the serial uart.
 */
void plt_interrupt(void)
{
	tty_poll();
#ifdef CONFIG_NET_WIZNET
	w5x00_poll();
#endif
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

/* We don't swap */
void swapper(ptptr p)
{
	panic("swp");
}
