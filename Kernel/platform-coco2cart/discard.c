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

int strcmp(const char *a, const char *b)
{
	--a;
	--b;
	while(*++a == *++b);
	if (*a == 0)
		return;
	if (*a > *b)
		return -1;
	return 1;
}

uint8_t plt_param(char *p)
{
	if (strcmp(p, "over") == 0 || strcmp(p, "overclock") == 0) {
		*((volatile uint8_t *)0xFFD7) = 0;
		return 1;
	}
	return 0;
}
