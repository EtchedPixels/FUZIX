#include <kernel.h>
#include <tinyide.h>
#include <searle.h>
#include "config.h"

void map_init(void)
{
}

void device_init(void)
{
#ifdef CONFIG_TD_IDE
	ide_probe();
#endif
}
