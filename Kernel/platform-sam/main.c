#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>

uint16_t ramtop = PROGTOP;
uint8_t need_resched;

void platform_idle(void)
{
	__asm
		halt
	__endasm;
}

void do_beep(void)
{
}

__sfr __at 249 status;

void platform_interrupt(void)
{
#if 0
	if (status & 1)
		line_interrupt();
	if (status & 2)
		mouse_interrupt();
#endif
	if (status & 4) {
		timer_interrupt();
		kbd_interrupt();
	}
	/* FIXME: figure out how tty interrupt needs to be handled */
	tty_interrupt();
}

void platform_discard(void)
{
	/* Buffer handling can happen here */
}

