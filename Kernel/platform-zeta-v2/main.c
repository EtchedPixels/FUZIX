#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include "config.h"
#ifdef CONFIG_FLOPPY
#include "devfd.h"
#endif

extern unsigned char irqvector;

void platform_idle(void)
{
	/* Let's go to sleep while we wait for something to interrupt us;
	 * Makes the HALT LED go yellow, which amuses me greatly. */
	__asm
		halt
	__endasm;
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
		case 2:
			tty_interrupt();
			return;
		default:
			return;
	}
}
