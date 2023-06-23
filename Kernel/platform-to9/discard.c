#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <device.h>
#include <devtty.h>
#include <tinydisk.h>
#include <tinysd.h>
#include <sd.h>

void map_init(void)
{
}

void pagemap_init(void)
{
	pagemap_add(0x17);
	pagemap_add(0xE7);
	pagemap_add(0xA7);
	pagemap_add(0x67);
	pagemap_add(0x27);
	/* Extended pages if fitted - two bytes so need special
	   handling as actually F8 and then bits 6 and 2 of the 6946 port */
#if 0
	pagemap_add(0xF8);
	pagemap_add(0xF9);
	pagemap_add(0xFA);
	pagemap_add(0xFB);
#endif	
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
