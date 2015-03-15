/* string.c
 * Copyright (C) 1995,1996 Robert de Bath <rdebath@cix.compulink.co.uk>
 * This file is part of the Linux-8086 C library and is distributed
 * under the GNU Library General Public License.
 */

#include <string.h>

/********************** Function strlen ************************************/
size_t strlen(const char *str)
{
	const char* start = str;
	while (*str++)
		;
	return str - start;
}
