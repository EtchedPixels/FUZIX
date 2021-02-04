#include <stdint.h>
#include <eagle_soc.h>
#include "rom.h"
#include "globals.h"

extern int main(void);

void __main(void)
{
	/* See globals.h for the clock speeds. */

	/* Configure peripheral clock. */

	rom_i2c_writeReg(103, 4, 1, 0x88);
	rom_i2c_writeReg(103, 4, 2, 0x81); /* 189MHz */

	/* Configure CPU clock. */

	ets_update_cpu_frequency(CPU_CLOCK);
	PIN_PULLUP_DIS(PERIPHS_IO_MUX_U0TXD_U);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0TXD_U, FUNC_U0TXD);
	uart_div_modify(0, (PERIPHERAL_CLOCK * 1e6) / 115200);

	/* Enable SPI flash mapping. */

	SpiFlashCnfig(5, 4);
	SpiReadModeCnfig(5);
	Wait_SPI_Idle(sdk_flashchip);
	Cache_Read_Enable(0, 0, 1);

	/* Clear BSS. */

	extern uint32_t _bss_start;
	extern uint32_t _bss_end;
	for (uint32_t* p = &_bss_start; p < &_bss_end; p++)
		*p = 0;

	/* Call the main program. */

	main();
}

