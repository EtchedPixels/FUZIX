#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
//#include <blkdev.h>
#include <devtty.h>
#include <ports.h>

uint8_t kernel_flag = 1;
uint16_t swap_dev = 0xFFFF;

void plt_idle(void)
{
    irqflags_t flags = di();
    tty_poll();
    irqrestore(flags);
}

void do_beep(void)
{
}

/*
 * Map handling: allocate 3 banks per process
 */

void pagemap_init(void)
{
    int i;
    /* Add the user banks, taking care to land 4 as the last one as we
       use that for init (0/1/2/3 are the kernel) */
    for (i = 6; i >= 0; i--)
        pagemap_add(4 + i * 4);
}

void map_init(void)
{
}

uint8_t plt_param(char *p)
{
    return 0;
}

void device_init(void)
{
	//set 100Hz
        //IRQ-timer counts 900 ticks per sec
	port_irq_timer_target = 900 / TICKSPERSEC;
}

void plt_interrupt(void)
{
	tty_poll();
	// writing any value will ack IRQ and restart timer
	port_irq_timer_reset = 0;
	timer_interrupt();
}
