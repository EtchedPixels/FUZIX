#include <kernel.h>
#include <devhd.h>
#include <devtty.h>
#include <tty.h>
#include <kdata.h>
#include "devgfx.h"
#include <printf.h>
#include "genie.h"

void device_init(void)
{
#ifdef CONFIG_RTC
	/* Time of day clock */
	inittod();
#endif
	floppy_setup();
	hd_probe();
	tty_probe();
	gfx_init();
}

void map_init(void)
{
}

/* Each RAM192 card provides four banks. For simplicty in the banking
   code we use the byte value for port 0x7E to select the bank as the
   bank number */
void pagemap_init(void)
{
	if (card_map == 0)
		panic("No RAM192 found.");
	if (card_map & 1) {
		pagemap_add(0x01);
		pagemap_add(0x11);
		pagemap_add(0x21);
		pagemap_add(0x31);
	}
	if (card_map & 2) {
		pagemap_add(0x05);
		pagemap_add(0x15);
		pagemap_add(0x25);
		pagemap_add(0x35);
	}
	if (card_map & 4) {
		pagemap_add(0x09);
		pagemap_add(0x19);
		pagemap_add(0x29);
		pagemap_add(0x39);
	}
	if (card_map & 8) {
		pagemap_add(0x0D);
		pagemap_add(0x1D);
		pagemap_add(0x2D);
		pagemap_add(0x3D);
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
	return 0;
}
