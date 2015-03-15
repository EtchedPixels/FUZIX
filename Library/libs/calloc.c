/* Copyright (C) 1995,1996 Robert de Bath <rdebath@cix.compulink.co.uk>
 * This file is part of the Linux-8086 C library and is distributed
 * under the GNU Library General Public License.
 *
 * This is a combined alloca/malloc package. It uses a classic algorithm
 * and so may be seen to be quite slow compared to more modern routines
 * with 'nasty' distributions.
 */  
    
#include "malloc-l.h"

void *calloc(size_t elm, size_t sz) 
{
	register size_t v = elm * sz;
	register void *ptr = malloc(v);
	
	if (ptr)
		memset(ptr, 0, v);
	return ptr;
}
