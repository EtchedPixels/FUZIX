#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <timer.h>
#include <devtty.h>
#include <z80retro.h>

uint16_t swap_dev = 0xFFFF;

void plt_discard(void)
{
	/* TODO: discard up to 0xC000 and fill with buffers, especially
	   as our I/O is slow */
}

void plt_idle(void)
{
	__asm halt __endasm;
}

uint8_t plt_param(unsigned char *p)
{
	used(p);
	return 0;
}

void plt_interrupt(void)
{
	static uint8_t n;
	tty_drain_sio();
	n++;
	if (n == 24) {
		timer_interrupt();
		n = 0;
	}
}
