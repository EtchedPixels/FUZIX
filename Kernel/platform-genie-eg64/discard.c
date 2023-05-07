#include <kernel.h>
#include <devhd.h>
#include <devtty.h>
#include <tty.h>
#include <kdata.h>
#include "trs80.h"

void vt_check_lower(void)
{
  *VT_BASE = 'a';
  if (*VT_BASE == 'a')
    video_lower = 1;
}

void device_init(void)
{
#ifdef CONFIG_RTC
  /* Time of day clock */
  inittod();
#endif
  hd_probe();
  trstty_probe();
}

void map_init(void)
{
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
