#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <device.h>
#include <devtty.h>
#include <tinydisk.h>
#include <tinysd.h>
#include <sd.h>
#include <devinput.h>

void map_init(void)
{
}

void pagemap_init(void)
{
	/* Assumes 512K for the moment */
	uint8_t i;
	uint8_t pmax = ramsize >> 4;
	for (i = 0x06; i < pmax ; i+= 0x02)
		pagemap_add(i);
	/* Somewhere to put this */
	i = *(volatile uint8_t *)0x6074;
	i &= 0xC0;
	if (i == 0x40)
		kputs("Light pen detected.\n");
	else if (i == 0xC0)
		kputs("Mouse detected.\n");
	inputdev = i & 0xC0;
}

void to_sd_probe(void)
{
	kputs("Probing SDDrive\n");
	sd_type = SDIF_SDDRIVE;
	sd_probe();
#if 0
	/* Need to debug these and double check their 6846 interactions etc */
	sdmoto_init();
	kputs("Probing SDMoto\n");
	sd_type = SDIF_SDMOTO;
	do_sd_probe();
	kputs("Probing SDMo\n");
	sd_type = SDIF_SDMO;
	do_sd_probe();
#endif
}
