#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <device.h>
#include <devtty.h>
#include <carts.h>
#include <blkdev.h>

/* Unlike the bigger ports we don't do cartridge management or MPI handling due to our memory
   tightness */
static const char *sysname[] = {"Dragon", "COCO", "COCO3", "Unknown"};

void map_init(void)
{
	kprintf("%s system.\n", sysname[system_id]);
}
