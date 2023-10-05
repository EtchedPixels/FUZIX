#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <timer.h>
#include <devtty.h>
#include <2063.h>

struct blkbuf *bufpool_end = bufpool + NBUFS;	/* minimal for boot -- expanded after we're done with _DISCARD */
uint16_t ramtop = 0x7E00;
uint8_t vdptype;

void plt_discard(void)
{
	while (bufpool_end + 1 < (struct blkbuf *) KERNEL_TOP) {
		memset(bufpool_end, 0, sizeof(struct blkbuf));
#if BF_FREE != 0
		bufpool_end->bf_busy = BF_FREE;	/* redundant when BF_FREE == 0 */
#endif
		bufpool_end->bf_dev = NO_DEVICE;
		bufpool_end++;
	}
	kprintf("%d disk buffers, ending at %p\n", bufpool_end - bufpool, bufpool_end);
}

void plt_idle(void)
{
	__asm
		halt
	__endasm;
}

uint8_t plt_param(unsigned char *p)
{
	used(p);
	return 0;
}

static int16_t timerct;

/*
 *	TODO:
 *	We need to switch this to IM2 to get the CTC useable properly
 *	Also a chance to switch to the im2 sio code for neatness and
 *	speed.
 */

/* Call timer_interrupt at 180Hz (close to - might need a tiny fudge */
static void timer_tick(void)
{
	timerct += 1;
	if (timerct >= 18) {
		timer_interrupt();
		timerct = 0;
	}
}

void plt_interrupt(void)
{
	extern uint8_t sd_count;
	tty_drain_sio();
	poll_input();
	/* Handle ticks lost due to SD / GPIO contention */
	if (sd_count) {
		while(sd_count) {
			timer_tick();
			sd_count--;
		}
	}
	timer_tick();
}

