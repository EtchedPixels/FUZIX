#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>

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
	timer_interrupt();
}

void map_init(void)
{
}

/* Toggle bit 1, set the no ROM bits */
#define PAGE_MAP(page)	(((page) | 0x04)^0x02)

/* Kernel in 0/1. We don't pull video tricks with 0xF000 yet to save memory
   but we should eventually */

void pagemap_init(void)
{
	/* This is model dependent */
	pagemap_add(PAGE_MAP(2));
	pagemap_add(PAGE_MAP(3));
#if defined(CONFIG_UB256) || defined(CONFIG_UB512)
	pagemap_add(PAGE_MAP(0) | 0x40);
	pagemap_add(PAGE_MAP(1) | 0x40);
	pagemap_add(PAGE_MAP(2) | 0x40);
	pagemap_add(PAGE_MAP(3) | 0x40);
#endif
#if defined(CONFIG_UB256TC) || defined(CONFIG_UB512)
	pagemap_add(PAGE_MAP(0) | 0x80);
	pagemap_add(PAGE_MAP(1) | 0x80);
	pagemap_add(PAGE_MAP(2) | 0x80);
	pagemap_add(PAGE_MAP(3) | 0x80);
#endif
#if defined(CONFIG_UB512)
	pagemap_add(PAGE_MAP(0) | 0xC0);
	pagemap_add(PAGE_MAP(1) | 0xC0);
	pagemap_add(PAGE_MAP(2) | 0xC0);
	pagemap_add(PAGE_MAP(3) | 0xC0);
#endif
/* and if we ever poke at the 1024 stuff its bit 7-5/1-0 */
}

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
