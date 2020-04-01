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

uint8_t platform_tick_present;

uint16_t rtc_port;
uint8_t rtc_shadow;

void platform_discard(void)
{
	while (bufpool_end < (struct blkbuf *) (KERNTOP - sizeof(struct blkbuf))) {
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
	if (ctc_present)
		__asm halt __endasm;
	else {
		irqflags_t irq = di();
		sync_clock();
		irqrestore(irq);
	}
}

uint8_t platform_param(unsigned char *p)
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

void platform_interrupt(void)
{
	if (acia_present)
		tty_pollirq_acia();
	if (sio_present)
		tty_pollirq_sio0();
	if (ctc_present) {
		uint8_t n = 255 - CTC_CH3;
		CTC_CH3 = 0x47;
		CTC_CH3 = 255;
		timer_tick(n);
	}
}
