#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <tinydisk.h>
#include <tinysd.h>

extern void sd_spi_clock(bool fast);

void map_init(void)
{
}

void device_init(void)
{
	sd_shift[0] = *((uint8_t *)0x89FF) == 3 ? 1 : 9;
	sd_spi_clock(1);
	/* Install a single tinysd interface */
	td_register(sd_xfer, 1);
}
