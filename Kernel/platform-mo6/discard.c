#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <device.h>
#include <devtty.h>
#include <blkdev.h>

void map_init(void)
{
}

void pagemap_init(void)
{
	/* 128K fixed for now */
	/* Video and kernel in 0, 1 is pinned user, 2 and 3 kernel */
	pagemap_add(0x04);
	pagemap_add(0x06);
}
