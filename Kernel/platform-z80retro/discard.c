#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <tinydisk.h>
#include <tinysd.h>
#include <z80retro.h>
#include "config.h"

/* Everything in here is discarded after init starts */

void init_hardware_c(void)
{
	ramsize = 512;
	procmem = 512 - 64;
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
	uint8_t r = sd_init();
	if (r == 0)
		return;
	sd_shift[0] = (r & CT_BLOCK) ? 0 : 9;
	td_register(sd_xfer, 1);
}
