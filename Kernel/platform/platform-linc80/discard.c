#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <tinyide.h>
#include <tinysd.h>
#include "config.h"

extern void pio_setup(void);

void map_init(void)
{
}

void device_init(void)
{
        pio_setup();
#ifdef CONFIG_TD_IDE
	ide_probe();
#endif
#ifdef CONFIG_TDSD
	sd_probe();
#endif
}
