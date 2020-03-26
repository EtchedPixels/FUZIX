/* string.c
 * Copyright (C) 1995,1996 Robert de Bath <rdebath@cix.compulink.co.uk>
 * This file is part of the Linux-8086 C library and is distributed
 * under the GNU Library General Public License.
 */

#include <string.h>

/********************** Function strrchr ************************************/
char *strrchr(const char *s, int c)
{
	register const char *p = s + strlen(s);

	/* For null it's just like strlen */
	if (c == '\0')
		return (char *)p;
	while (p != s) {
		if (*--p == c)
			return (char *)p;
	}
	return NULL;
}
