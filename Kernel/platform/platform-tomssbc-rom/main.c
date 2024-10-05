#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>

uint16_t ramtop = PROGTOP;
struct blkbuf *bufpool_end = bufpool + NBUFS;	/* minimal for boot -- expanded after we're done with _DISCARD */
uint16_t swap_dev = 0xFFFF;

void plt_discard(void)
{
        uint8_t *end = (uint8_t *)0xF000;
        end -= sizeof(struct blkbuf);
	while (bufpool_end < (struct blkbuf *)end) {
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
  __asm
   halt
  __endasm;
}

void plt_interrupt(void)
{
	/* Will call timer_interrupt as needed */
	tty_pollirq_sio0();
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

