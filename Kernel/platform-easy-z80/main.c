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

uint8_t ctc_present;

void platform_discard(void)
{
}

void platform_idle(void)
{
	if (ctc_present)
		__asm halt __endasm;
	else {
		irqflags_t irq = di();
		sync_clock();
		irqrestore(irq);
	}
}

uint8_t platform_param(unsigned char *p)
{
	used(p);
	return 0;
}

static int16_t timerct;

/* Call timer_interrupt at 10Hz */
static void timer_tick(uint8_t n)
{
	timerct += n;
	while (timerct >= 16) {
		timer_interrupt();
		timerct -= 16;
	}
}

void platform_interrupt(void)
{
	uint8_t n = 255 - CTC_CH3;
	tty_drain_sio();
	CTC_CH3 = 0x47;
	CTC_CH3 = 255;
	timer_tick(n);
}
