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
uint8_t ctc_present;

/* Real time clock support for DS1302 driver - port and other pins */
uint16_t rtc_port = 0xC0;
uint8_t rtc_shadow;

#define CTC_CH0	0x88
#define CTC_CH1	0x89
#define CTC_CH2	0x8A
#define CTC_CH3	0x8B

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
 */

static int16_t timerct;

/* Call timer_interrupt at 10Hz */
static void timer_tick(uint8_t n)
{
	timerct += n;
	while (timerct >= 20) {
		timer_interrupt();
		timerct -= 20;
	}
}

void plt_interrupt(void)
{
	tty_poll();
	if (ctc_present) {
		uint8_t n = 255 - in(CTC_CH3);
		out(CTC_CH3, 0x47);
		out(CTC_CH3, 0xFF);
		timer_tick(n);
	}
#ifdef CONFIG_NET_W5100
	w5x00_poll();
#endif
#ifdef CONFIG_NET_W5300
	w5300_poll();
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
	extern uint16_t _code;
	uint16_t discard_size = (uint16_t)_code - (uint16_t)bufpool_end;
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
