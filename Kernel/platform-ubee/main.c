#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <ubee.h>

uint16_t ramtop = PROGTOP;

/* On idle we spin checking for the terminals. Gives us more responsiveness
   for the polled ports */
void platform_idle(void)
{
	__asm halt __endasm;
}

void do_beep(void)
{
}

void platform_interrupt(void)
{
	kbd_interrupt();
	timer_interrupt();
}

void map_init(void)
{

}

/* The bank bits are not laid out nicely for historical reasons

   A classic uBee has the bank selection in bits 0/1
   The 256TC needed an extra bit so reused bit 5 (rom selector)
   The 3rd party 512K/1MB add ons used bits 7/6 for the high bank bits

   Bank 0/0 holds the kernel and common Bank 0/1 holds the kernel low 32k */
static uint8_t map_mod(uint8_t banknum)
{
	uint8_t r = banknum & 0x03;
	r |= 0xC;		/* ROM off, Video off */
	if (ubee_model == UBEE_256TC)
		r |= (banknum & 4) ? 0x20 : 0;
	else
		r |= (banknum & 0x0C) << 4;
	return r;
}

/* Kernel in bank 0 low/high,user apps in bank 1 low/high (and if present
   other banks too). Memorywise it's a lot like the TRS80 layout */

void pagemap_init(void)
{
	uint8_t i;
	uint8_t nbank = procmem / 32;
	for (i = 1; i < nbank; i++)
		pagemap_add(map_mod(i));

}

/* FIXME: check RTC is not an option on supported devices, if so probe it */
__sfr __at 0x04 rtc_c;
__sfr __at 0x06 rtc_d;

uint8_t rtc_secs(void)
{
	rtc_c = 0x00;
	return rtc_d;
}

uint8_t platform_param(char *p)
{
	return 0;
}
