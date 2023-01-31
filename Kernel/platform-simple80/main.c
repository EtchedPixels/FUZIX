#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <timer.h>
#include <devtty.h>
#include <rtc.h>
#include <ds1302.h>
#include <rcbus.h>

extern unsigned char irqvector;
struct blkbuf *bufpool_end = bufpool + NBUFS;	/* minimal for boot -- expanded after we're done with _DISCARD */
uint16_t swap_dev = 0xFFFF;
uint16_t ramtop = 0xF000;
uint8_t need_resched = 0;

uint8_t ctc_present;

uint8_t plt_tick_present;

uint16_t rtc_port = 0xC0;
uint8_t rtc_shadow;

void plt_discard(void)
{
	while (bufpool_end < (struct blkbuf *) (KERNTOP - sizeof(struct blkbuf))) {
		memset(bufpool_end, 0, sizeof(struct blkbuf));
#if BF_FREE != 0
		bufpool_end->bf_busy = BF_FREE;	/* redundant when BF_FREE == 0 */
#endif
		bufpool_end->bf_dev = NO_DEVICE;
		bufpool_end++;
	}
	kprintf("Buffers available: %d\n", bufpool_end - bufpool);
}

void plt_idle(void)
{
	if (ctc_present)
		__asm halt __endasm;
	else {
		irqflags_t irq = di();
		sync_clock();
		irqrestore(irq);
	}
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
	tty_pollirq();
	if (ctc_present) {
		uint8_t n = 255 - CTC_CH3;
		CTC_CH3 = 0x47;
		CTC_CH3 = 255;
		timer_tick(n);
	}
}
