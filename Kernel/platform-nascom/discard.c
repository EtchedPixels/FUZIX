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
#ifdef CONFIG_RTC
	/* Time of day clock */
	// FIXME : once we merge both versions of the RTC handling inittod();
#endif
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
