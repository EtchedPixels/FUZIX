#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <timer.h>
#include <devtty.h>
#include <devfd.h>
#include <rtc.h>
#include <ds1302.h>
#include <rcbus.h>

extern unsigned char irqvector;
struct blkbuf *bufpool_end = bufpool + NBUFS;	/* minimal for boot -- expanded after we're done with _DISCARD */
uint16_t swap_dev = 0xFFFF;
uint16_t ramtop = 0x7E00;
uint8_t ctc_present;
uint8_t sio_present;
uint8_t sio1_present;
uint8_t mach_zrcc;
uint8_t plt_tick_present;

uint16_t rtc_port = 0xC0;
uint8_t rtc_shadow;

void plt_discard(void)
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

void plt_idle(void)
{
	irqflags_t irq = di();
	/* Check the clock. We try and reduce the impact of the clock on
	   latency by not doing it so often. 256 may be too small a divide
	   need to see what 1/10th sec looks like in poll loops */
	tty_poll_cpld();
	irqrestore(irq);

	if (!idlect++)
		sync_clock();
}

uint8_t plt_param(unsigned char *p)
{
	used(p);
	return 0;
}

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
	tty_poll_cpld();
	if (sio_present)
		tty_pollirq_sio0();
	if (sio1_present)
		tty_pollirq_sio1();
	if (ctc_present) {
		uint8_t n = 255 - CTC_CH3;
		CTC_CH3 = 0x47;
		CTC_CH3 = 255;
		timer_tick(n);
	}
}

