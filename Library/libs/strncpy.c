/* string.c
 * Copyright (C) 1995,1996 Robert de Bath <rdebath@cix.compulink.co.uk>
 * This file is part of the Linux-8086 C library and is distributed
 * under the GNU Library General Public License.
 */  
    
#include <string.h>
    
/********************** Function strncpy ************************************/ 
char *strncpy(char *d, const char *s, size_t l) 
{
	register char *s1 = d;
	register const char *s2 = s;

	while (l) {
		l--;
		if ((*s1++ = *s2++) == '\0')
			break;
	}

	/* This _is_ correct strncpy is supposed to zap */ 
	while (l-- != 0)
		*s1++ = '\0';
	return d;
}


