/* string.c
 * Copyright (C) 1995,1996 Robert de Bath <rdebath@cix.compulink.co.uk>
 * This file is part of the Linux-8086 C library and is distributed
 * under the GNU Library General Public License.
 */  
    
#include <string.h>
    
/********************** Function strcpy ************************************/ 
#undef strcpy
char *strcpy(char *d, const char *s) 
{
	return memcpy(d, s, strlen(s) + 1);
}
