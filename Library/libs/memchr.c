/* memchr.c
 * Copyright (C) 1995,1996 Robert de Bath <rdebath@cix.compulink.co.uk>
 * This file is part of the Linux-8086 C library and is distributed
 * under the GNU Library General Public License.
 */  
    
#include <string.h>
    
/********************** Function memchr ************************************/ 
void *memchr(const void *str, int c, size_t l) 
{
	register const char *p = str;

	while (l-- != 0) {
		if (*p == c)
			return (void*) p;
		p++;
	}
	return NULL;
}
