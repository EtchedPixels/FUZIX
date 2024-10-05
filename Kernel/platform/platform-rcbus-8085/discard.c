#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <tinyide.h>
#include <tinyscsi.h>
#include <ch375.h>

/*
 *	The MMU map bits are arranged as
 *
 *	A16L A16H A17L A17H A18L A18H A19L A19H and resets to 0 (ROM)
 *	on boot. We are entered with MMU mapping 3 and we use this
 *	as our kernel
 *
 *	Our user mappings then become (using high and low of banks)
 *	C3 33 F3 0F CF 3F FF
 */

void pagemap_init(void)
{
	/* Low 0 is kernel map, high 0 unused except at boot */
	pagemap_add(0xC3);
	pagemap_add(0x33);
	pagemap_add(0xF3);
	pagemap_add(0x0F);
	pagemap_add(0xCF);
	pagemap_add(0x3F);
	/* This one weill be picked up by init and must be first to
	   match crt0.S */
	pagemap_add(0xFF);
}

/* Nothing to do for the map of init */
void map_init(void)
{
}

uint_fast8_t plt_param(char *p)
{
	return 0;
}


void device_init(void)
{
	ds1302_init();
	ppide_init();
	ide_probe();
	scsi_init();
	ch375_probe();
}
