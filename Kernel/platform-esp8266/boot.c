#include <stdint.h>
#include "rom.h"
#include "globals.h"
#include "esp8266_peri.h"

extern int main(void);

extern uint32_t *platform_vectors;

void __main(void)
{
	uint32_t addr;

	/* Warp emgines on - takes us to 160MHz (doesn't affect the peripheral clock) */
	CPU2X |= 1;

	uart_div_modify(0, (PERIPHERAL_CLOCK * 1e6) / 115200);
	Cache_Read_Enable(0, 0, 1);

	/* Clear BSS. */

	extern uint32_t _bss_start;
	extern uint32_t _bss_end;
	for (uint32_t* p = &_bss_start; p < &_bss_end; p++)
		*p = 0;

	/* Steal the interrupt vectors */
	asm volatile ("wsr.ps %0" :: "a" (0x2F));
	asm volatile ("wsr.vecbase %0" :: "a" (&platform_vectors));
	asm volatile ("rsync");

	/* Call the main program. */

	main();
}

/* vim: sw=4 ts=4 et: */

