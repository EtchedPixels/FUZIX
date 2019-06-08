#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <tty.h>
#include <devtty.h>
#include <rtc.h>
#include <bios.h>

uint16_t ramtop = PROGTOP;
uint16_t swap_dev = 0xFFFF;

struct fuzixbios_info *biosinfo;

/*
 *	This routine is called continually when the machine has nothing else
 *	it needs to execute. On a machine with entirely interrupt driven
 *	hardware this could just halt for interrupt.
 */
void platform_idle(void)
{
	fuzixbios_idle();
}

/*
 *	Called when the timer ticks. We should extend this to scale the
 *	timer nicely. Flags tells us useful things like if we are a vblank
 *	or a 1/10th tick etc
 */
uint16_t callback_tick(uint16_t event) __z88dk_fastcall
{
	if (event == TICK_TIMER)
		timer_interrupt();
	return 0;
}

uint16_t callback_tty(uint16_t val) __z88dk_fastcall
{
	uint8_t tty = val;
	uint8_t op = tty >> 4;
	tty &= 0x0F;
	switch(op) {
	case 0:
		tty_inproc(tty, val >> 8);
		break;
	case 1:
		tty_carrier_raise(tty);
		break;
	case 2:
		tty_carrier_drop(tty);
		break;
	}
	return 0;
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
	return fuzixbios_rtc_secs();
}
