#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <timer.h>
#include <tty.h>
#include <devtty.h>
#include <rtc.h>

struct blkbuf *bufpool_end = bufpool + NBUFS;	/* minimal for boot -- expanded after we're done with _DISCARD */
uint16_t swap_dev = 0xFFFF;
uint16_t ramtop = 0xE000;
uint8_t need_resched = 0;

uint8_t plt_tick_present;

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
}

extern uint8_t keycheck(void);

static void poll_keyboard(void)
{
	uint8_t k = keycheck();
	if (k)
		tty_inproc(1, k);
}

void plt_idle(void)
{
	irqflags_t irq = di();
	poll_keyboard();
	irqrestore(irq);
}

uint8_t plt_param(unsigned char *p)
{
	used(p);
	return 0;
}

/* CTC calls in here 8 times a second */
void plt_interrupt(void)
{
	static uint8_t c;
	++c;
	c &= 7;
	poll_keyboard();
	/* Produce a 10Hz timer */
	if (c == 4 || c == 7)
		timer_interrupt();
	timer_interrupt();
}

void do_beep(void)
{
	/* for now */
}

/*
 *	So that we don't suck in a library routine we can't use from
 *	the runtime
 */

size_t strlen(const char *p)
{
	size_t len = 0;
	while(*p++)
		len++;
	return len;
}

