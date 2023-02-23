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

void pagemap_init(void)
{
	int i;

	/* Add the pages not in use by the kernel */
	for (i = 36; i < 64; i++)
		pagemap_add(i);

	/* finally add the common area */
/* 	pagemap_add(32 + 3); */
	/* Swap this over when we go to all RAM booting from firmware */
	pagemap_add(33);
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
