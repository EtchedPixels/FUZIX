#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>

uint16_t ramtop = PROGTOP;
uint8_t vtattr_cap;
uint16_t swap_dev;

struct blkbuf *bufpool_end = bufpool + NBUFS;

/* On idle we spin checking for the terminals. Gives us more responsiveness
   for the polled ports */
void platform_idle(void)
{
	irqflags_t irq = di();
	tty_poll();
	irqrestore(irq);
}

void do_beep(void)
{
}

uint8_t platform_param(char *p)
{
	used(p);
	return 0;
}

void platform_interrupt(void)
{
	/* TODO */
	kbd_poll();
	tty_poll();
	timer_interrupt();
}

/*
 *	FIXME: reclaim to end of usable memory
 */
void platform_discard(void)
{
	bufptr bp = bufpool_end;
	extern uint16_t discard_size;

	discard_size /= sizeof(struct blkbuf);

	kprintf("%d buffers reclaimed from discard\n", discard_size);

	bufpool_end += discard_size;	/* Reclaim the discard space */

	memset(bp, 0, discard_size * sizeof(struct blkbuf));
	/* discard_size is in discard so it dies here */
	for (bp = bufpool + NBUFS; bp < bufpool_end; ++bp) {
		bp->bf_dev = NO_DEVICE;
		bp->bf_busy = BF_FREE;
	}
}
