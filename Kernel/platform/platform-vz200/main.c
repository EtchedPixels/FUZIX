#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <timer.h>
#include <tty.h>
#include <devtty.h>
#include <rtc.h>

uint16_t swap_dev = 0xFFFF;
uint16_t ramtop = 0xFFF4;	/* TODO */
uint8_t need_resched = 0;
uint8_t vtrows = 16;
uint8_t vtbottom = 15;

extern uint8_t postinit;

extern void video_switch(void);
extern void interrupt_setup(void);

void plt_discard(void)
{
	/* Now discard is gone we can finally set up our interrupt handler */
	interrupt_setup();
	/* Switch video if the 8K video banking is available */
	video_switch();
}

void plt_idle(void)
{
	irqflags_t irq = di();
	tty_pollirq(!postinit);
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

