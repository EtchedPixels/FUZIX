#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <devfd.h>
#include <devinput.h>

extern unsigned char irqvector;
struct blkbuf *bufpool_end = bufpool + NBUFS;	/* minimal for boot -- expanded after we're done with _DISCARD */

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

void platform_idle(void)
{
	/* FIXME: for the non CTC case we should poll the DS1302 here and
	   fake up appopriate timer events */
	/* Let's go to sleep while we wait for something to interrupt us;
	 * Makes the HALT LED go yellow, which amuses me greatly. */
//	__asm halt __endasm;
	irqflags_t irq = di();
	tty_pollirq_sio();
	irqrestore(irq);
}

uint8_t platform_param(unsigned char *p)
{
	used(p);
	return 0;
}

void platform_interrupt(void)
{
	if (1) { 	/* CTC == 0 when we do the work */
#ifdef CONFIG_FLOPPY
		fd_tick();
#endif
//		poll_input();
//		timer_interrupt();
	}
	tty_pollirq_sio();
	return;
}
