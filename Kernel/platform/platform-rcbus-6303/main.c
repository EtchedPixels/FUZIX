#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <blkdev.h>
#include <devide.h>
#include <devtty.h>

uint8_t kernel_flag = 1;
uint8_t need_resched;
uint16_t swap_dev = 0xFFFF;

/* Onboard I/O */
static volatile uint8_t *cpuio = (volatile uint8_t *)0;

void plt_idle(void)
{
    irqflags_t flags = di();
    tty_poll();
    irqrestore(flags);
}

void do_beep(void)
{
}


/* FIXME: route serial interrupts directly in asm to a buffer and just
   queue process here */

static uint8_t tickmod;

void plt_interrupt(void)
{
	uint8_t dummy;

	tty_poll();

	/* Reading the register resets the flag if set */
	if (cpuio[0x08] & 0x20) {
		cpuio[9];	/* Read tmer to clear interrupt */
		tickmod++;
		if (tickmod == 7)
			tickmod = 0;
		/* Turn it into a 20Hz timer. For now do a simple fixup
		   for the 28->20 and ignore the extra 1% or so error */
		if (tickmod != 2 && tickmod != 6)
			timer_interrupt();
	}
}

extern uint8_t hd_map;
extern uint8_t hd_page;
extern void hd_read_data(uint8_t *p);
extern void hd_write_data(uint8_t *p);

void devide_read_data(void)
{
	hd_map = blk_op.is_user;
	hd_page = blk_op.swap_page;
	hd_read_data(blk_op.addr);	
}

void devide_write_data(void)
{
	hd_map = blk_op.is_user;
	hd_page = blk_op.swap_page;
	hd_write_data(blk_op.addr);	
}
