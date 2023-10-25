#include <kernel.h>
#include <devtty.h>
#include <devscsi.h>
#include <printf.h>
#include <blkdev.h>
#include <tty.h>
#include <nascom.h>

uint8_t io_page;
uint16_t bankmap;

void device_init(void)
{
	unsigned i;
	/* TODO: rtc init proper */
	kputs("Checking for IDE interface on PIO\n");
	ide_pio_setup();
	ide_probe();
	/* Check for a RAMdisc */
	if (gm833_probe() && swap_dev == 0xFFFF) {
		/* If we found no swap use ramdisc 0 as 512K of swap */
		swap_dev = 0x0800;
		for (i = 0; i < 10; i++)
			swapmap_add(i * 48);
		kputs("Using /dev/rd0 GM833 for swap\n");
	}
}

void map_init(void)
{
}

/* Add pagemap codes depending upon the present banks */
void pagemap_init(void)
{
	/* We didn't find any extra page mode memory */
	if (bankmap < (2 << 8))
		panic("no pagemode");
	/* Add the present banks (may be non contiguous). The
	   first bank is the kernel and not available */
	if (bankmap & 2)
		pagemap_add(0x22);
	if (bankmap & 4)
		pagemap_add(0x44);
	if (bankmap & 8)
		pagemap_add(0x88);
}
