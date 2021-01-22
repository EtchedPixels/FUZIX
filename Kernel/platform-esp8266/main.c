#include <stdint.h>
#include <eagle_soc.h>
#include "kernel.h"

extern void ets_putc(char c);
extern void fuzix_main(void);

uaddr_t ramtop = PROGTOP;
uint8_t need_resched;

static void puts(const char* s)
{
	for (;;)
	{
		char c = *s++;
		if (!c)
			break;
		ets_putc(c);
	}
}

void map_init(void) {}
void platform_discard(void) {}

uint_fast8_t platform_param(char* p)
{
	return 0;
}

void platform_reboot(void)
{
	panic("platform_reboot()");
}

void platform_monitor(void)
{
	platform_reboot();
}

void device_init(void)
{
}

int main(void)
{
	for (int i=0; i<10; i++)
		puts("...");

	di();
	fuzix_main();
}

