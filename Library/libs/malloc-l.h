/* Copyright (C) 1995,1996 Robert de Bath <rdebath@cix.compulink.co.uk>
 * This file is part of the Linux-8086 C library and is distributed
 * under the GNU Library General Public License.
 *
 * This is a combined alloca/malloc package. It uses a classic algorithm
 * and so may be seen to be quite slow compared to more modern routines
 * with 'nasty' distributions.
 */
#include <types.h>
#include <malloc.h>
#include <errno.h>
#include <syscalls.h>
#include <string.h>

#define __MINI_MALLOC__

#define MCHUNK		512	/* Allocation unit in 'mem' elements */
#undef LAZY_FREE		/* If set frees can be infinitly defered */
#undef MINALLOC	/* 32 */	/* Smallest chunk to alloc in 'mem's */
#undef VERBOSE 			/* Lots of noise, debuging ? */

#undef	malloc
#define MAX_INT ((int)(((unsigned)-1)>>1))

#ifdef VERBOSE
#define noise __noise
#else
#define noise(y,x)
#endif

typedef struct mem_cell {
	struct mem_cell *next;	/* A pointer to the next mem */
	unsigned int size;	/* An int >= sizeof pointer */
	char *depth;		/* For the alloca hack */
} mem;

#define m_size(p)  ((p)[0].size)		/* For malloc */
#define m_next(p)  ((p)[0].next)		/* For malloc and alloca */
#define m_deep(p)  ((p)[0].depth)		/* For alloca */
#define m_add(x,y) (mem *)((uchar *)x + y)	/* Sum mem* with y bytes */

extern void *__mini_malloc __P((size_t));
extern void *(*__alloca_alloc) __P((size_t));
extern mem *__freed_list;
