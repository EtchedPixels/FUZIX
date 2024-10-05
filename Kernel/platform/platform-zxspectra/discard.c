#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <blkdev.h>

extern uint8_t fuller, kempston, kmouse, kempston_mbmask;

void pagemap_init(void)
{
	uint8_t i;
	pagemap_add(4);
	pagemap_add(6);
}

/* string.c
 * Copyright (C) 1995,1996 Robert de Bath <rdebath@cix.compulink.co.uk>
 * This file is part of the Linux-8086 C library and is distributed
 * under the GNU Library General Public License.
 */
int strcmp(const char *d, const char *s)
{
	register char *s1 = (char *) d, *s2 = (char *) s, c1, c2;

	while ((c1 = *s1++) == (c2 = *s2++) && c1);
	return c1 - c2;
}

uint8_t plt_param(char *p)
{
	if (strcmp(p, "kempston") == 0) {
		kempston = 1;
		return 1;
	}
	if (strcmp(p, "kmouse") == 0) {
		kmouse = 1;
		return 1;
	}
	if (strcmp(p, "fuller") == 0) {
		fuller = 1;
		return 1;
	}
	if (strcmp(p, "kmouse3") == 0) {
		kmouse = 1;
		kempston_mbmask = 7;
		return 1;
	}
	if (strcmp(p, "kmturbo") == 0) {
		/* For now rely on the turbo detect - may want to change this */
		kmouse = 1;
		return 1;
	}
	return 0;
}

/* Nothing to do for the map of init */
void map_init(void)
{
}

void plt_copyright(void)
{
}
