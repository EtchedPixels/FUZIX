#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <blkdev.h>
#include <devide.h>
#include <devsd.h>
#include "config.h"
#include <z180.h>
#include <netdev.h>
#include <ps2kbd.h>
#include <ps2mouse.h>
#include "z180itx.h"

uint16_t romwbw_cpu;	/* Updated by bootstram asm */
uint16_t romwbw_speed;
uint16_t romwbw_type;


/* Mini-ITX 82C55 */
__sfr __at 0x40 ppi_a;
__sfr __at 0x41 ppi_b;
__sfr __at 0x42 ppi_c;
__sfr __at 0x43 ppi_ctrl;

void init_hardware_c(void)
{
	/* Set up so that when we flip the control bits we stay in the
	   right addressing etc */
	ppi_a = 0xFF;
	ppi_c = 0x0F;
	/* A is output, B is input, mode 0 C lower is output, C higher
	   is input */
	ppi_ctrl = 0x8A;
	/* ROM off */
	ppi_a = 0xF7;

	ramsize = 1024;
	procmem = 1024 - 64;

	/* zero out the initial bufpool */
	memset(bufpool, 0, (char *) bufpool_end - (char *) bufpool);
}

void pagemap_init(void)
{
	int i;

	/* Upper RAM: starts with the kernel */
	for (i = 0x91; i < (1024 >> 2); i += 0x10)
		pagemap_add(i);
	/* Lower RAM: all usable */
	for (i = 0x01; i < (512 >> 2); i += 0x10)
		pagemap_add(i);

	ps2kbd_present = ps2kbd_init();
	if (ps2kbd_present) {
		kputs("PS/2 Keyboard at 0x78/0x7A\n");
#if 0
		if (tms9918a_present) {
			/* Add the consoles */
			uint8_t n = 0;
			shadowcon = 0;
			kputs("Switching to video output.\n");
			do {
				insert_uart(0x98, &tms_uart);
				n++;
			} while(n < 4 && nuart <= NUM_DEV_TTY);
		}
#endif
	}
	ps2mouse_present = ps2mouse_init();
	if (ps2mouse_present) {
		kputs("PS/2 Mouse at 0x78/0x7A\n");
		/* TODO: wire to input layer and interrupt */
	}
}

void map_init(void)
{
	/* clone udata and stack into a regular process bank, return with common memory
	   for the new process loaded */
	copy_and_map_process(&init_process->p_page);
	/* kernel bank udata (0x300 bytes) is never used again -- could be reused? */
}

uint8_t plt_param(char *p)
{
	used(p);
	return 0;
}

void device_init(void)
{
	devide_init();
	devsd_init();
#ifdef CONFIG_NET
	netdev_init();
#endif
}
