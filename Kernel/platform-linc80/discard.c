#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <devide.h>
#include <devsd.h>
#include <blkdev.h>
#include "config.h"

extern void pio_setup(void);

void map_init(void)
{
}

void device_init(void)
{
        pio_setup();
#ifdef CONFIG_IDE
	devide_init();
#endif
#ifdef CONFIG_SD
	devsd_init();
#endif
}
