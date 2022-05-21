#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>

uint16_t ramtop = PROGTOP;
uint8_t need_resched;

void plt_idle(void)
{
	__asm
		halt
	__endasm;
}

void do_beep(void)
{
}

__sfr __at 249 status;

void plt_interrupt(void)
{
	uint8_t r = status;
#if 0
	if (r & 1)
		line_interrupt();
	if (r & 2)
		mouse_interrupt();
	if (r & 4)
		midi_interrupt();
#endif
	if (r & 8) {
		timer_interrupt();
		kbd_interrupt();
	}
	/* FIXME: figure out how tty interrupt needs to be handled */
	tty_interrupt();
}

void plt_discard(void)
{
	/* Buffer handling can happen here */
}

