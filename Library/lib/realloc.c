/* Copyright (C) 1995,1996 Robert de Bath <rdebath@cix.compulink.co.uk>
 * This file is part of the Linux-8086 C library and is distributed
 * under the GNU Library General Public License.
 *
 * This is a combined alloca/malloc package. It uses a classic algorithm
 * and so may be seen to be quite slow compared to more modern routines
 * with 'nasty' distributions.
 */  
    
#include "malloc-l.h"

void *realloc(void *ptr, size_t size) 
{
	void *nptr;
	unsigned int osize;
	if (ptr == 0)
		return malloc(size);
	
	/* ??? what if I really want to free rest of block ? */ 
	if (size <= (osize = (m_size(((mem *) ptr) - 1) - 1) * sizeof(mem)))
		return ptr;
	if ((nptr = malloc(size)) == NULL)
		return 0;
	memcpy(nptr, ptr, osize);
	free(ptr);
	return nptr;
}


