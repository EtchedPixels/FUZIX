/*
 *	Indexed data. We keep various random little vectors attached to
 *	things in order to track stuff like array sizes and struct fields
 *
 *	This manages them
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "compiler.h"

static unsigned idxmem[IDX_SIZE];
static unsigned *idxptr = idxmem;
/*
 *	Simple for now - we may well be able to brk() this pool although
 *	we will have to decide if we fix the pool or the symbol table size..
 */
unsigned *idx_get(unsigned len)
{
    /* len is in words */
    unsigned *p = idxptr;
    idxptr += len;
    if (idxptr >= idxmem + IDX_SIZE)
        fatal("out of index memory");
    return p;
}

unsigned *idx_copy(unsigned *p, unsigned n)
{
    unsigned *r = idx_get(n);
    memcpy(r, p, n * sizeof(unsigned));
    return r;
}
