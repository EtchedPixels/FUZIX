#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <tinyide.h>
#include <tom.h>
#include "config.h"

void map_init(void)
{
}

void device_init(void)
{
	ide_probe();
}
