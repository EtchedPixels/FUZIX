#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <tinyide.h>
#include <lb.h>
#include "ncr5380.h"

void map_init(void)
{
	/* TODO: set up CTC timer */
}

void device_init(void)
{
    scsi_init();
}
