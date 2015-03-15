/* string.c
 * Copyright (C) 1995,1996 Robert de Bath <rdebath@cix.compulink.co.uk>
 * This file is part of the Linux-8086 C library and is distributed
 * under the GNU Library General Public License.
 */  
    
#include <string.h>
    
/********************** Function strcat ************************************/ 
char *strcat(char *d, const char *s) 
{
	strcpy(d + strlen(d), s);
	return d;
}
