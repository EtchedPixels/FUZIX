#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <tinyide.h>

uint_fast8_t plt_param(unsigned char *p)
{
	return 0;
}

void map_init(void)
{
}

void pagemap_init(void)
{
	pagemap_add(0x01);
	pagemap_add(0x02);
}

void device_init(void)
{
	ide_probe();
}
