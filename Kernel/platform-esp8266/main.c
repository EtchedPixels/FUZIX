#include <stdint.h>
#include <eagle_soc.h>
#include "esp8266_peri.h"
#include "kernel.h"
#include "kdata.h"

extern void ets_putc(char c);
extern void fuzix_main(void);
extern void sd_rawinit(void);

volatile uint32_t* const esp8266_gpioToFn[16] = { &GPF0, &GPF1, &GPF2, &GPF3, &GPF4, &GPF5, &GPF6, &GPF7, &GPF8, &GPF9, &GPF10, &GPF11, &GPF12, &GPF13, &GPF14, &GPF15 };

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
void platform_monitor(void) {}
void platform_reboot(void) {}

uint_fast8_t platform_param(char* p)
{
	return 0;
}

void device_init(void)
{
}

int main(void)
{
	ramsize = 80;
	procmem = 64;

	di();
	fuzix_main();
}

