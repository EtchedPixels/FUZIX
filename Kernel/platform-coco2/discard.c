#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <device.h>
#include <devtty.h>
#include <blkdev.h>

static const char *sysname[] = {"Dragon", "COCO", "COCO3", "Unknown"};

void map_init(void)
{
	uint8_t i;

	kprintf("%s system.\n", sysname[system_id]);
}
