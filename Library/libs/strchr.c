/* string.c
 * Copyright (C) 1995,1996 Robert de Bath <rdebath@cix.compulink.co.uk>
 * This file is part of the Linux-8086 C library and is distributed
 * under the GNU Library General Public License.
 */

#include <string.h>

/* FIXME: asm version ?? */
/********************** Function strchr ************************************/
char *strchr(const char *s, int c)
{
	register char ch;

	for (;;) {
		if ((ch = *s) == c)
			return (char *)s;
		if (ch == 0)
			return 0;
		s++;
	}
}
