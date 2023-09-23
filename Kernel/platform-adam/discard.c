#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <ds1302.h>
#include <tinyide.h>
#include "config.h"
#include "adam.h"

void map_init(void)
{
}

void device_init(void)
{
	ide_probe();
	keypoll();	/* Kick off keyboard scanning */
	adamnet_probe();
}
