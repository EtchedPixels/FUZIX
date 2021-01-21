#include <stdint.h>
#include <eagle_soc.h>

extern void ets_putc(char c);
extern void ets_uart_printf(const char* format, ...);
extern void ets_update_cpu_frequency(int mhz);
extern void uart_div_modify(uint8_t uart_no, uint32_t divlatch);
extern void Cache_Read_Enable(uint8_t odd_even, uint8_t mb_count, uint8_t no_idea);

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

int main(void)
{
	puts("Hello, world!\n");
}

