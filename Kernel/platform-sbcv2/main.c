#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <rtc.h>
#include <ds1302.h>
#include <net_w5100.h>

uint16_t ramtop = PROGTOP;
uint16_t swap_dev = 0xFFFF;
uint8_t timermsr = 0;

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
void platform_idle(void)
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
void platform_interrupt(void)
{
	tty_poll();
#ifdef CONFIG_NET_WIZNET
	w5100_poll();
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
 *	Logic for tickless system. If you have an RTC with a timer tick
 *	you can ignore this.
 */

static uint8_t newticks = 0xFF;
static uint8_t oldticks;

static uint8_t re_enter;

/*
 *	Hardware specific logic to get the seconds. We really ought to enhance
 *	this to check minutes as well just in case something gets stuck for
 *	ages.
 */
static void sync_clock_read(void)
{
	uint8_t s;
	oldticks = newticks;
	ds1302_read_clock(&s, 1);
	s = (s & 0x0F) + (((s & 0xF0) >> 4) * 10);
	newticks = s;
}

/*
 *	The OS core will invoke this routine when idle (via platform_idle) but
 *	also after a system call and in certain other spots to ensure the clock
 *	is roughly valid. It may be called from interrupts, without interrupts
 *	or even recursively so it must protect itself using the framework
 *	below.
 *
 *	Having worked out how much time has passed in 1/10ths of a second it
 *	performs that may timer_interrupt events in order to advance the clock.
 *	The core kernel logic ensures that we won't do anything silly from a
 *	jump forward of many seconds.
 *
 *	We also choose to poll the ttys here so the user has some chance of
 *	getting control back on a messed up process.
 */
void sync_clock(void)
{
	if (!timermsr) {
		irqflags_t irq = di();
		int16_t tmp;
		if (!re_enter++) {
			sync_clock_read();
			if (oldticks != 0xFF) {
				tmp = newticks - oldticks;
				if (tmp < 0)
					tmp += 60;
				tmp *= 10;
				while(tmp--) {
					timer_interrupt();
				}
				/* Poll the PropIOv2 */
				platform_interrupt();
			}
		} 
		re_enter--;
		irqrestore(irq);
	}
}

/*
 *	This method is called if the kernel has changed the system clock. We
 *	don't work out how much work we need to do by using it as a reference
 *	so we don't care.
 */
void update_sync_clock(void)
{
}
