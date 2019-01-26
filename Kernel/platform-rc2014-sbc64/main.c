#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <timer.h>
#include <devtty.h>
#include <devfd.h>
#include <rtc.h>
#include <ds1302.h>

extern unsigned char irqvector;
struct blkbuf *bufpool_end = bufpool + NBUFS;	/* minimal for boot -- expanded after we're done with _DISCARD */
uint16_t swap_dev = 0xFFFF;
uint16_t ramtop = 0x7E00;
uint8_t ctc = 0;

void platform_discard(void)
{
	while (bufpool_end < (struct blkbuf *) ((uint16_t)&udata - sizeof(struct blkbuf))) {
		memset(bufpool_end, 0, sizeof(struct blkbuf));
#if BF_FREE != 0
		bufpool_end->bf_busy = BF_FREE;	/* redundant when BF_FREE == 0 */
#endif
		bufpool_end->bf_dev = NO_DEVICE;
		bufpool_end++;
	}
}

static uint8_t idlect;

void platform_idle(void)
{
	/* Check the clock. We try and reduce the impact of the clock on
	   latency by not doing it so often. 256 may be too small a divide
	   need t see what 1/10th sec looks like in poll loops */
	tty_poll_cpld();
	if (!idlect++)
		sync_clock();
}

uint8_t platform_param(unsigned char *p)
{
	used(p);
	return 0;
}

void platform_interrupt(void)
{
	tty_pollirq_sio();
}

/*
 *	Logic for tickless system. If you have an RTC you can ignore this.
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
	if (!ctc) {
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
