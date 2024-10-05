#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <timer.h>
#include <devtty.h>
#include <devfd.h>
#include <devinput.h>
#include <rtc.h>
#include <ds1302.h>
#include <easy-z80.h>

extern unsigned char irqvector;
uint16_t swap_dev = 0xFFFF;

uint8_t rtc_shadow;
uint16_t rtc_port = 0xC0;

void plt_discard(void)
{
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
	tty_drain_sio();
	timer_interrupt();
}
