#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <device.h>
#include <devtty.h>
#include <carts.h>
#include <blkdev.h>

uint8_t membanks;
uint8_t system_id;
uint16_t swap_dev;
struct blkbuf *bufpool_end = bufpool + NBUFS;

void platform_idle(void)
{
}

uint8_t platform_param(char *p)
{
	return 0;
}

void do_beep(void)
{
}

void platform_discard(void)
{
	extern uint8_t discard_size;
	bufptr bp = bufpool_end;

	kprintf("%d buffers reclaimed from discard\n", discard_size);

	bufpool_end += discard_size;	/* Reclaim the discard space */

	memset(bp, 0, discard_size * sizeof(struct blkbuf));
	/* discard_size is in discard so it dies here */
	for (bp = bufpool + NBUFS; bp < bufpool_end; ++bp) {
		bp->bf_dev = NO_DEVICE;
		bp->bf_busy = BF_FREE;
	}
}

