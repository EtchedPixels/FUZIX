#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <rtc.h>
#include <mbc2.h>

uint16_t ramtop = PROGTOP;
uint16_t swap_dev = 0xFFFF;
uint8_t has_rtc;

/*
 *	This routine is called continually when the machine has nothing else
 *	it needs to execute. On a machine with entirely interrupt driven
 *	hardware this could just halt for interrupt.
 */
void platform_idle(void)
{
	/* Everything we need is interrupt driven */
	__asm halt __endasm;
}

/*
 *	This routine is called from the interrupt handler code to process
 *	interrupts. All of the nasty stuff (register saving, bank switching,
 *	reti instructions) is dealt with for you.
 *
 *	Most platforms would read something to identify the interrupt source
 *	but in our case the only possible source is the serial uart.
 */
void platform_interrupt(void)
{
	uint8_t r;
	opcode = OP_GET_IRQ;
	r = opread;
	if (r & IRQ_CONSOLE)
		tty_poll();
	if (r & IRQ_TIMER)
		timer_interrupt();
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
void platform_discard(void)
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

/*
 *	Always called with interrupts off.
 */
uint8_t platform_rtc_secs(void)
{
	if (has_rtc) {
		opcode = OP_GET_RTC;
		/* Don't bother with the rest of the bytes */
		return opread;
	}
	return 255;
}
