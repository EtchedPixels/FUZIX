#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include "config.h"
#include <z180.h>

uint16_t ramtop = PROGTOP;
extern unsigned char irqvector;
uint16_t swap_dev = 0xFFFF;

uint16_t rtc_port = 0x0C;
uint8_t rtc_shadow = 0xAF;

struct blkbuf *bufpool_end = bufpool + NBUFS;	/* minimal for boot -- expanded after we're done with _DISCARD */

void platform_discard(void)
{
	while (bufpool_end <
	       (struct blkbuf *) (KERNTOP - sizeof(struct blkbuf))) {
		memset(bufpool_end, 0, sizeof(struct blkbuf));
#if BF_FREE != 0
		bufpool_end->bf_busy = BF_FREE;	/* redundant when BF_FREE == 0 */
#endif
		bufpool_end->bf_dev = NO_DEVICE;
		bufpool_end++;
	}
}

static uint8_t light = 0xF0;
static uint8_t lightdir = 1;
static uint8_t lightct = 0;

__sfr __at 0x0D led;

void z180_timer_interrupt(void)
{
	unsigned char a;

	/* we have to read both of these registers in order to reset the timer */
	a = TIME_TMDR0L;
	a = TIME_TCR;
	timer_interrupt();
	if (++lightct & 7)
		return;
	if (lightdir)
		light >>= 1;
	else
		light <<= 1;
	if (light == 0x0F)
		lightdir = 0;
	else if (light == 0xF0)
		lightdir = 1;
	led = light;
}

void platform_idle(void)
{
	/* Let's go to sleep while we wait for something to interrupt us */
	led = 0xFF;
	__asm halt __endasm;
}

void platform_interrupt(void)
{
	switch (irqvector) {
	case Z180_INT_TIMER0:
		z180_timer_interrupt();
		return;
	case Z180_INT_ASCI0:
		tty_pollirq_asci0();
		return;
	case Z180_INT_ASCI1:
		tty_pollirq_asci1();
		return;
	default:
		return;
	}
}

/* The RTC and GPIO share the same port so manage them together using
   the rtc shadow byte */

__sfr __at 0x0C gpio;

void gpio_set(uint8_t mask, uint8_t val)
{
	rtc_shadow &= ~mask;
	rtc_shadow |= val;
	gpio = rtc_shadow;
}

void platform_ds1302_setup(void)
{
}

void platform_ds1302_restore(void)
{
	gpio = rtc_shadow;
}
