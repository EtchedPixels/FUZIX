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

uint8_t fdc765_present;

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

void plt_idle(void)
{
	irqflags_t irq = di();
	tty_pollirq(0);
	irqrestore(irq);
}

uint8_t plt_param(unsigned char *p)
{
	used(p);
	return 0;
}

void plt_interrupt(void)
{
	/* Do the video first, we can muck around with timers and task
	   switching outside of vblank and nobody will care */
	tty_pollirq(1);
	timer_interrupt();
}

void do_beep(void)
{
}

unsigned char vt_mangle_6847(unsigned char c)
{
	if (c >= 96) {
		c -= 32;
		c &= 0x3F;
		c |= 0x40;	/* Invert to show caps etc */
	} else
		c &= 0x3F;
	return c;
}
