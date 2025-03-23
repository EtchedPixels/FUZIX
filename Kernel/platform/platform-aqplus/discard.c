#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <iop.h>
#include "config.h"

/* Everything in here is discarded after init starts */
/* We are entered with text mode on, turbo on, in 80 column mode */
void init_hardware_c(void)
{
	uint8_t tmp[16];
	uint8_t c;

	ramsize = 512;
	procmem = 512 - 64;

	/* Clean up any loader handles etc */
	iop_cmd(0x1F);	/* CLOSEALL */
	iop_rx();
	/* Keyboard to ASCII for now */
	iop_cmd(0x08);
	iop_data(0x07);
	/* Interrupt on vblank */
	out(0xEE, 0x01);

	iop_cmd(0x02);
	while(c = iop_rx())
		kputchar(c);
	kputchar('\n');
}

/* Kernel is at 0x3C-0x3F, boot common is at 0x3F */
void pagemap_init(void)
{
	int i;

	/* Add the pages not in use by the kernel */
	for (i = 0x20; i < 0x3C; i++)
		pagemap_add(i);
	/* finally add the common area */
	pagemap_add(0x3F);
}

void map_init(void)
{
}

void device_init(void)
{
	iop_probe();
}
