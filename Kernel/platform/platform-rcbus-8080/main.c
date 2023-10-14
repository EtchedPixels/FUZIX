#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <blkdev.h>
#include <devfdc765.h>
#include <rcbus.h>

uaddr_t ramtop = PROGTOP;
uint16_t swap_dev = 0xFFFF;

static int16_t timerct;
static uint8_t vblank;
uint8_t acia_present;
uint8_t tms9918a_present;
uint8_t uart_present;
uint8_t rtc_shadow;

void plt_interrupt(void)
{
	tty_poll();

	/* If the TMS9918A is present prefer it as a time source */
	if (tms9918a_present) {
		if (tms_interrupt());
			vblank++;
		while(vblank >= 6) {
			/* TODO vblank wakeup for gfx */
			timer_interrupt();
			poll_input();
			vblank -= 6;
		}
	} else {
		/* If not then use the 82C54 */
		if (timer_check()) {
			timer_interrupt();
			poll_input();
		}
	}
}

void plt_reinterrupt(void)
{
	if (tms_interrupt())
		vblank++;
	tty_poll();
}
