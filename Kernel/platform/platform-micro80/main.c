#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <timer.h>
#include <devtty.h>
#include <micro80.h>

struct blkbuf *bufpool_end = bufpool + NBUFS;	/* minimal for boot -- expanded after we're done with _DISCARD */
uint16_t swap_dev = 0xFFFF;
uint16_t ramtop = 0xFFFF;
uint8_t need_resched = 0;

void plt_discard(void)
{
	while (bufpool_end < (struct blkbuf *) (0xFFFF - sizeof(struct blkbuf))) {
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
	__asm halt __endasm;
}

uint8_t plt_param(unsigned char *p)
{
	used(p);
	return 0;
}

void plt_interrupt(void)
{
	uint8_t n = 255 - CTC_CH3;
	tty_drain_sio();
	CTC_CH3 = 0x47;
	CTC_CH3 = 255;
	while(n > 0) {
		timer_interrupt();
		n--;
	}
}
