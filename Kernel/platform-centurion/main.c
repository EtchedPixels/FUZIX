#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <blkdev.h>
#include <devtty.h>

uint8_t kernel_flag = 1;
uint8_t need_resched;
uint16_t swap_dev = 0xFFFF;

/* Onboard I/O */
static volatile uint8_t *cpuio = (volatile uint8_t *)0;

void platform_idle(void)
{
    irqflags_t flags = di();
    tty_poll();
    irqrestore(flags);
}

/* TODO once we understand it */
void platform_interrupt(void)
{
	tty_poll();
	timer_interrupt();
}
