#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <blkdev.h>
#include <tinysd.h>

void map_init(void)
{
}

void device_init(void)
{
	sd_shift[0] = *((uint8_t *)0x89FF) == 3 ? 1 : 9;
	sd_spi_clock(1);
	sd_setup(0);
}
