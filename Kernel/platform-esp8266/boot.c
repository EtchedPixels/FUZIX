#include <stdint.h>
#include <eagle_soc.h>

extern void ets_putc(char c);
extern void ets_uart_printf(const char* format, ...);
extern void ets_update_cpu_frequency(int mhz);
extern void uart_div_modify(uint8_t uart_no, uint32_t divlatch);
extern void Cache_Read_Enable(uint8_t odd_even, uint8_t mb_count, uint8_t no_idea);

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

	/* Call the main program. */

	main();
}


