#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <blkdev.h>
#include <devfdc765.h>
#include <rcbus.h>

uaddr_t ramtop = PROGTOP;
uint16_t swap_dev = 0xFFFF;

static int16_t timerct;
static uint8_t vblank;
uint8_t acia_present;
uint8_t tms9918a_present;
uint8_t uart_present;
uint8_t rtc_shadow;

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
	/* Stay below E000 for when we start using the upper 8K switching */
	uint16_t discard_size = 0xE000 - (uint16_t)bufpool_end;
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

void plt_interrupt(void)
{
	tty_poll();

	/* If the TMS9918A is present prefer it as a time source */
	if (tms9918a_present) {
		if (tms_interrupt());
			vblank++;
		while(vblank >= 6) {
			/* TODO vblank wakeup for gfx */
			timer_interrupt();
			poll_input();
			vblank -= 6;
		}
	} else {
		/* If not then use the 82C54 */
		if (timer_check()) {
			timer_interrupt();
			poll_input();
		}
	}
}

void plt_reinterrupt(void)
{
	if (tms_interrupt())
		vblank++;
	tty_poll();
}
