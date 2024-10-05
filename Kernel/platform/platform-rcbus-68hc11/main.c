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

/* RTC bit copy */
uint8_t rtc_shadow = 0;

/* Onboard I/O */

void plt_idle(void)
{
    irqflags_t flags = di();
//    tty_poll();
    irqrestore(flags);
}

void do_beep(void)
{
}


/* FIXME: route serial interrupts directly in asm to a buffer and just
   queue process here */

/* Our E clock is 1.8432MHz, the prescaler divides it by 8 giving us
   230400 timer clocks per second which for 50Hz is 4608 timer clocks */
#define CLK_PER_TICK	4608

uint16_t timer_step = CLK_PER_TICK;

/* This is incremented at 50Hz by the asm code */
uint8_t timer_ticks;

/* Each timer event we get called after the asm code has set up the timer
   again. ticks will usually be incremented by one each time, but if we
   miss a tick the overflow logic may make ticks larger */
void plt_event(void)
{
	tty_poll();

	while(timer_ticks >= 5) {
		timer_interrupt();
		timer_ticks -= 5;
	}
}

extern uint8_t hd_map;
extern uint8_t hd_swap;
extern void hd_read_data(uint8_t *p);
extern void hd_write_data(uint8_t *p);

void devide_read_data(void)
{
	hd_swap = blk_op.swap_page;
	if (blk_op.is_user)
		hd_map = 1;
	else
		hd_map = 0;
	hd_read_data(blk_op.addr);	
}

void devide_write_data(void)
{
	hd_swap = blk_op.swap_page;
	if (blk_op.is_user)
		hd_map = 1;
	else
		hd_map = 0;
	hd_write_data(blk_op.addr);	
}
