#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <timer.h>
#include <devtty.h>
#include <devfd.h>
#include <rtc.h>
#include <ds1302.h>
#include <rc2014.h>

struct blkbuf *bufpool_end = bufpool + NBUFS;	/* minimal for boot -- expanded after we're done with _DISCARD */
uint16_t swap_dev = 0xFFFF;
uint16_t ramtop = 0xC000;
uint8_t need_resched = 0;

uint8_t acia_present;
uint8_t ctc_present;
uint8_t sio_present;

uint8_t plt_tick_present;

uint16_t rtc_port = 0xC0;
uint8_t rtc_shadow;

void plt_discard(void)
{
	/* Tricky - need to avoid overflow */
	while (bufpool_end <= (struct blkbuf *)(0xFFFFU - sizeof(struct blkbuf))) {
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
	/* Check the clock. We try and reduce the impact of the clock on
	   latency by not doing it so often. 256 may be too small a divide
	   need t see what 1/10th sec looks like in poll loops */
	if (ctc_present)
		plt_halt();
	else {
		irqflags_t irq = di();
		sync_clock();
		irqrestore(irq);
	}
}

uint_fast8_t plt_param(unsigned char *p)
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
	if (acia_present)
		tty_pollirq_acia();
	if (sio_present)
		tty_pollirq_sio0();
	if (ctc_present) {
		uint8_t n = 255 - in(CTC_CH3);
		out(CTC_CH3, 0x47);
		out(CTC_CH3, 255);
		timer_tick(n);
	}
}
