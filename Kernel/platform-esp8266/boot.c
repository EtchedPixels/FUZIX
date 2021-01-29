#include <stdint.h>
#include <eagle_soc.h>
#include "rom.h"

extern int main(void);

void __main(void)
{
	/* Set up the UART and system clock frequency. */

	ets_update_cpu_frequency(52);
	PIN_PULLUP_DIS(PERIPHS_IO_MUX_U0TXD_U);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0TXD_U, FUNC_U0TXD);
	uart_div_modify(0, 52e6 / 115200);

	/* Enable SPI flash mapping. */

	Cache_Read_Enable(0, 0, 1);

	/* Clear BSS. */

	extern uint32_t _bss_start;
	extern uint32_t _bss_end;
	for (uint32_t* p = &_bss_start; p < &_bss_end; p++)
		*p = 0;

#if 0
	/* Set up the vector table. */

	extern uint32_t platform_vector_table;
	asm volatile ("wsr.vecbase %0" :: "a" (&platform_vector_table));
#endif

	/* Call the main program. */

	main();
}


