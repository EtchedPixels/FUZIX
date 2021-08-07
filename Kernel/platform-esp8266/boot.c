#include <stdint.h>
#include "rom.h"
#include "globals.h"

extern int main(void);

extern uint32_t *platform_vectors;

void __main(void)
{
	uint32_t addr;
	/* See globals.h for the clock speeds. */
	/* Configure peripheral clock. */
#if 0
	rom_i2c_writeReg(103, 4, 1, 0x88);
	//rom_i2c_writeReg(103, 4, 2, 0x81); /* 189MHz */
	rom_i2c_writeReg(103, 4, 2, 0x91); /* 80MHz */

	/* Configure CPU clock. */
	ets_update_cpu_frequency(CPU_CLOCK);
#endif
	uart_div_modify(0, (PERIPHERAL_CLOCK * 1e6) / 115200);
#if 0
	/* Enable SPI flash mapping. */
	SpiFlashCnfig(5, 0);
	SpiReadModeCnfig(5);
	Wait_SPI_Idle(sdk_flashchip);
#endif
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
	asm volatile ("rsr.vecbase %0" : "=a" (addr));
	kprintf("VT %p\n", addr);

	/* Call the main program. */

	main();
}

/* vim: sw=4 ts=4 et: */

