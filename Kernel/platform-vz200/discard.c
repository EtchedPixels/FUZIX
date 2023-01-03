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
	sd_setup(0);
}
