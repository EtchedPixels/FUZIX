#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <tinyide.h>
#include <devvd.h>
#include <devfd.h>
#include <devtty.h>
#include <pcw8256.h>

/* Kernel is 0-3 screen for now is 4 and bits of 5
   Apps 6,7,8,9,10,11,12,13,14,15 etc
   
   This *may* change once we sort the memory map out sanely so that we are
   only using 4 for kernel/screen. More likely though we will instead use
   the split bank for buffers ? */

void map_init(void)
{
	udata.u_ptab->p_page = 0x8686;
	udata.u_ptab->p_page2 = 0x8686;
}

__sfr __at 0xFC daisy;

void pagemap_init(void)
{
	uint8_t i;
	uint8_t top = ramsize / 16;
	for (i = 0x07; i < top; i++)
		pagemap_add(i | 0x80);
}

uint8_t plt_param(char *p)
{
	used(p);
	return 0;
}

const char *mname[] = {
	"PCW8256",
	"PCW8512",
	"PCW9512",
	"PCW9256/PCW10",
	"PCW9512PLUS"
};

__sfr __at 0xFD printstat;

void device_init(void)
{
	fd_probe();
	if (is_joyce)
		devvd_probe();
	else
		ide_probe();
}

void machine_ident(void)
{
	if (new_fdc == 0) {
		if (printstat & 0x20)
			model = MODEL_PCW9512;
		else if (ramsize == 256)
			model = MODEL_PCW8256;
		else
			model = MODEL_PCW8512;
	} else {
		if (ramsize > 256)
			model = MODEL_PCW9512PLUS;
		else
			model = MODEL_PCW9256;	/* Or 10 - basically identical */
	}
	kprintf("Model: %s%s\n", mname[model], is_joyce?"(joyce)":"");
}
