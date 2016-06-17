/* Copyright (C) 1995,1996 Robert de Bath <rdebath@cix.compulink.co.uk>
 * This file is part of the Linux-8086 C library and is distributed
 * under the GNU Library General Public License.
 *
 * Kept from the old malloc package.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void *calloc(size_t elm, size_t sz)
{
	/* FIXME: a modern calloc should probably trap overflows and NULL */
	size_t v = elm * sz;
	void *ptr = malloc(v);

	if (ptr)
		memset(ptr, 0, v);
	return ptr;
}
