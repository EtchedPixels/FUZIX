/* string.c
 * Copyright (C) 1995,1996 Robert de Bath <rdebath@cix.compulink.co.uk>
 * This file is part of the Linux-8086 C library and is distributed
 * under the GNU Library General Public License.
 */

#include "string.h"
#include <ctype.h>

/********************** Function stricmp ************************************/
int stricmp(const char *s, const char *d)
{
	for (;;) {
		unsigned char sc = *(const uchar *) s++, dc = *(const uchar *) d++;

		if (sc != dc) {
			if (tolower(sc) != tolower(dc))
				return (int) (char) (sc - dc);
		} else if (sc == '\0')
			break;
	}
	return 0;
}
