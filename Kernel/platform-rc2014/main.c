#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include "config.h"
#ifdef CONFIG_FLOPPY
#include <devfd.h>
#endif

extern unsigned char irqvector;
struct blkbuf *bufpool_end = bufpool + NBUFS; /* minimal for boot -- expanded after we're done with _DISCARD */

void platform_discard(void)
{
    while(bufpool_end < (struct blkbuf*)(KERNTOP - sizeof(struct blkbuf))){
        memset(bufpool_end, 0, sizeof(struct blkbuf));
#if BF_FREE != 0
        bufpool_end->bf_busy = BF_FREE; /* redundant when BF_FREE == 0 */
#endif
        bufpool_end->bf_dev = NO_DEVICE;
        bufpool_end++;
    }
}

void platform_idle(void)
{
	/* Let's go to sleep while we wait for something to interrupt us;
	 * Makes the HALT LED go yellow, which amuses me greatly. */
	__asm
		halt
	__endasm;
}

uint8_t platform_param(unsigned char *p)
{
    used(p);
    return 0;
}

void platform_interrupt(void)
{
	switch(irqvector) {
		case 1:
#ifdef CONFIG_PPP
			tty_poll_ppp()
#endif
#ifdef CONFIG_FLOPPY
			fd_tick();
#endif
			timer_interrupt();
			return;

		case 4:
#ifdef CONFIG_SIO
			tty_pollirq_sio();
#endif
			return;
                case 0x38:
#ifdef CONFIG_ACIA
                        tty_pollirq_acia();
#endif
                        return;
		default:
			return;
	}
}
