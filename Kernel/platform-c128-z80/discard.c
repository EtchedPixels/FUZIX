#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <blkdev.h>
#include <devsd.h>
#include "config.h"

extern void rd_probe(void);

void map_init(void)
{
}

void device_init(void)
{
	rd_probe();
#ifdef CONFIG_SD
	devsd_init();
#endif
}
