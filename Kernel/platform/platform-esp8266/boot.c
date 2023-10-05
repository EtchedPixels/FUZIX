#include <kernel.h>

#include "rom.h"
#include "globals.h"
#include "esp8266_peri.h"
#include "kernel-esp8266.def"

/*
 *	Everything in this file is loaded into iRAM and discarded when we start user space
 */
extern int main(void);

extern uint32_t *platform_vectors;

extern uint32_t _bss_start;
extern uint32_t _bss_end;

void __main(void)
{
	asm volatile ("movi a1, swapstack_end");
	asm volatile ("call0 _main");
}

void _main(void)
{
	uint32_t *p;

	/* Warp emgines on - takes us to 160MHz (doesn't affect the peripheral clock) */
	CPU2X |= 1;

	uart_div_modify(0, (PERIPHERAL_CLOCK * 1e6) / 115200);
	Cache_Read_Enable(0, 0, 0);

	/* Clear BSS. */

	for (p = &_bss_start; p < &_bss_end; p++)
		*p = 0;

#ifdef DO_MEM_CHECK
	/* Move the buffers back before using this test */
	uint32_t flap;
	/* A small block at the start is used for various things like the SPI lock */
	/* Pattern all the areas we believe safe to use */

	flap = *(uint32_t *)0x3FFFC714;
	kprintf("Flash pointer is %p\n", flap);
	p = (uint32_t *)0x3FFFC040;
	while(p < (uint32_t *)0x40000000) {
		/* Flash configuration structure */
		if (p == (uint32_t *)0x3FFFC714) {
			p += 8;		/* 24 bytes of used space */
			continue;
		}
		*p = 0x1DEC1DED;
		p++;
	}
#endif
	/* Steal the interrupt vectors */
	asm volatile ("wsr.ps %0" :: "a" (0x2F));
	asm volatile ("wsr.vecbase %0" :: "a" (&platform_vectors));
	asm volatile ("rsync");

	/* Call the main program. */

	main();
}

/* vim: sw=4 ts=4 et: */

