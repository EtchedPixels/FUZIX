#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>

extern uint8_t fuller, kempston, kmouse, kempston_mbmask;

void pagemap_init(void)
{
	uint8_t i;
	uint8_t r;

	pagemap_add(3);
	pagemap_add(4);

	/* This is good to 512K on the Pentagon 1024/Kay 1024. We will need
	   to rethink stuff for the upper 512K if we ever care */
	/* Bits 2-0 are written to 7FFD with the right ROM info */
	/* Bits 7-4 are written to 1FFD with the right ROM disable etc */

	/* FIXME: for now hardcode 256K */
	for (i = 8; i < 16; i++) {
		r = i & 7;
		if (i & 8)
			r |= 0x10;	/* bank bit 3 */
		if (i & 16)
			r |= 0x80;
		/* Scorpion 1024K mod only
		if (i & 32)
			r |= 0x40;
		*/
		pagemap_add(r);
	}
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

/* Nothing special needed for NemoIDE reset at boot */
void ide_reset(void)
{
	ide_std_reset();
}
