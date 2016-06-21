/* string.c
 * Copyright (C) 1995,1996 Robert de Bath <rdebath@cix.compulink.co.uk>
 * This file is part of the Linux-8086 C library and is distributed
 * under the GNU Library General Public License.
 */

#include "string.h"
#include <ctype.h>

/********************** Function strnicmp ************************************/

int strnicmp(const char *s, const char *d, size_t l)
{
	while (l-- != 0) {
		unsigned char sc = *(const uchar *) s++, dc = *(const uchar *) d++;

		if (sc != dc) {
			if (tolower(sc) != tolower(dc))
				return (int) (char) (sc - dc);
		} else if (sc == '\0')
			break;
	}
	return 0;
}
