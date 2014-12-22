#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <devrtc.h>

__sfr __at 25 clkc;
__sfr __at 26 clkd;


void zrtc_init(void)
{
	clkc = 0xFF;
	clkc = 0x00;
	inittod();
}

uint8_t rtc_secs(void)
{
	return clkd;
}
