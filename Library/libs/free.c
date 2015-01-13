/* Copyright (C) 1995,1996 Robert de Bath <rdebath@cix.compulink.co.uk>
 * This file is part of the Linux-8086 C library and is distributed
 * under the GNU Library General Public License.
 *
 * This is a combined alloca/malloc package. It uses a classic algorithm
 * and so may be seen to be quite slow compared to more modern routines
 * with 'nasty' distributions.
 */

#include "malloc-l.h"

/* Start the alloca with just the dumb version of malloc */
void *(*__alloca_alloc) __P((size_t)) = __mini_malloc;

/* the free list is a single list of free blocks. __freed_list points to
   the highest block (highest address) and each block points to the lower
   block (lower address). last block points to 0 (initial value of
   _freed_list)
*/
mem *__freed_list = 0;

#ifdef VERBOSE
/* NB: Careful here, stdio may use malloc - so we can't */
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
static void pstr __P((char *));
static void phex __P((unsigned));
static void noise __P((char *, mem *));
static void pstr(char *str)
{
	write(2, str, strlen(str));
} static void phex(unsigned val)
{
	char buf[8];
	strcpy(buf, "000");
	ltoa((long) val, buf + 3, 16);
	pstr(buf + strlen(buf + 4));
} void __noise(char *y, mem * x)
{
	pstr("Malloc ");
	phex((unsigned) x);
	pstr(" sz ");
	phex(x ? (unsigned) m_size(x) : 0);
	pstr(" nxt ");
	phex(x ? (unsigned) m_next(x) : 0);
	pstr(" is ");
	pstr(y);
	pstr("\n");
}
#endif				/*  */
void free(void *ptr)
{
	register mem *top, *chk = (mem *) ptr;
	if (chk == 0)
		return;		/* free(NULL) - be nice */
	chk--;
      try_this:;
	top = (mem *) sbrk(0);
	if (m_add(chk, m_size(chk)) >= top) {
		noise("FREE brk", chk);
		brk((void *) ((uchar *) top - m_size(chk)));

		/* Adding this code allow free to release blocks in any order;
		 * they can still only be allocated from the top of the heap
		 * tho.
		 */
#ifdef __MINI_MALLOC__
                /* FIXME: void * cast appears to be a cc65 bug */
		if (__alloca_alloc == (void *)__mini_malloc && __freed_list) {
			chk = __freed_list;
			__freed_list = m_next(__freed_list);
			goto try_this;
		}
#endif				/*  */
	}

	else {			/* Nope, not sure where this goes, leave it for malloc to deal with */

#ifdef __MINI_MALLOC__
		/* check if block is already on free list.
		   if it is, return without doing nothing */
		top = __freed_list;
		while (top) {
			if (top == chk)
				return;
			top = m_next(top);
		}

		/* else add it to free list */
		if (!__freed_list || chk > __freed_list) {

			/* null free list or block above free list */
			m_next(chk) = __freed_list;
			__freed_list = chk;
		}

		else {

			/* insert block in free list, ordered by address */
			register mem *prev = __freed_list;
			top = __freed_list;
			while (top && top > chk) {
				prev = top;
				top = m_next(top);
			}
			m_next(chk) = top;
			m_next(prev) = chk;
		}

#else				/*  */
		m_next(chk) = __freed_list;
		__freed_list = chk;

#endif				/*  */
		noise("ADD LIST", chk);
	}
}

void *__mini_malloc(size_t size)
{
	register mem *ptr;
	register unsigned int sz;

	/* First time round this _might_ be odd, But we won't do that! */
#if 0
	sz = (unsigned int) sbrk(0);
	if (sz & (sizeof(struct mem_cell) - 1)) {
		if (sbrk
		    (sizeof(struct mem_cell) -
		     (sz & (sizeof(struct mem_cell) - 1))) < 0)
			goto nomem;
	}
#endif				/*  */
	if (size == 0)
		return 0;

	/* Minor oops here, sbrk has a signed argument */
	if (size > (((unsigned) -1) >> 1) - sizeof(struct mem_cell) * 3) {
	      nomem:errno = ENOMEM;
		return 0;
	}
	size += sizeof(struct mem_cell);	/* Round up and leave space for size field */
	ptr = (mem *) sbrk(size);
	if ((int) ptr == -1)
		return 0;
	m_size(ptr) = size;
	noise("CREATE", ptr);
	return ptr + 1;
}
