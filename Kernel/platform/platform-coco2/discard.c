#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <device.h>
#include <devtty.h>
#include <blkdev.h>

static const char *sysname[] = {"Dragon", "COCO", "COCO3", "Unknown"};

extern uint16_t framedet;
extern uint8_t sys_hz;
extern uint8_t lower;

void map_init(void)
{
	if (framedet >= 0x0500)
		sys_hz = 5;
	else
		sys_hz = 6;
	kprintf("%d0Hz %s system.\n", sys_hz, sysname[system_id]);
}

int strcmp(const char *a, const char *b)
{
	--a;
	--b;
	while(*++a == *++b);
	if (*a == 0)
		return 0;
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
	if (strcmp(p, "t1") == 0) {
		lower = 1;
		*((volatile uint8_t *)0xFF22) |= 0x50;
	}
	return 0;
}
