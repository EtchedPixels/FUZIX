
#include "syshead.h"
#include "const.h"
#include "type.h"
#include "align.h"

static char NOMEMEORY[] = "Cannot allocate sufficient memory";

#ifdef USE_FIXED_HEAP
static char *heapend;		/* end of free space for symbol list */
static char *heapptr;		/* next free space in symbol list */
#endif

#ifndef USE_FIXED_HEAP
static char tempbuf[2048];
#endif

void init_heap(void)
{
#ifdef USE_FIXED_HEAP
#ifndef USERMEM
#define USERMEM 0xAC00U
#endif

#if defined(__AS386_16__) || defined(__m6809__)
    int stk;
    heapptr = sbrk(0);
    heapend = ((char*)&stk) - STAKSIZ - 16;
    brk(heapend);
    if(sbrk(0) != heapend)
       as_abort(NOMEMEORY);
#else
    heapptr = malloc(USERMEM);
    heapend = heapptr + USERMEM;
    if (heapptr == 0)
	as_abort(NOMEMEORY);
#endif
#endif
}

void *temp_buf(void)
{
#ifdef USE_FIXED_HEAP
    return heapptr;
#else
    return tempbuf;
#endif
}

void *asalloc(unsigned int size)
{
    void * rv;
#ifdef USE_FIXED_HEAP
    align(heapptr);
    if (heapptr+size < heapend)
    {
        rv = heapptr;
        heapptr += size;
    }
    else
       rv = 0;
#else
    rv = malloc(size);
#endif
    if (rv == 0 && size) as_abort(NOMEMEORY);
    return rv;
}


void *asrealloc(void *oldptr, unsigned int size)
{
    void * rv;
#ifdef USE_FIXED_HEAP
    if (oldptr == 0) return asalloc(size);

    if ((char*)oldptr+size < heapend)
    {
        heapptr = (char*)oldptr + size;
        rv = oldptr;
    }
    else
        rv = 0;
#else
    rv = realloc(oldptr, size);
#endif

    if (rv == 0) as_abort(NOMEMEORY);
    return rv;
}

