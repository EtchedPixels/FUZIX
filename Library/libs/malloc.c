/* Copyright (C) 1995,1996 Robert de Bath <rdebath@cix.compulink.co.uk>
 * This file is part of the Linux-8086 C library and is distributed
 * under the GNU Library General Public License.
 *
 * This is a combined alloca/malloc package. It uses a classic algorithm
 * and so may be seen to be quite slow compared to more modern routines
 * with 'nasty' distributions.
 */

#include "malloc-l.h"

/* The chunk_list pointer is either NULL or points to a chunk in a
 * circular list of all the free blocks in memory
 */

#define Static static
static mem *chunk_list = 0;
Static mem *__search_chunk __P((unsigned));
Static void __insert_chunk __P((mem *));
void *malloc(size_t size)
{
	register unsigned sz;
	register mem *ptr = 0;
	if (size == 0)
		return 0;	/* ANSI STD */

	/* Ensure size is aligned, otherwise our memory nodes become unaligned
	 * and we get hard-to-debug errors on platforms which require
	 * aligned accesses. */
	size = (size + sizeof(void*) - 1) & ~(sizeof(void*) - 1);

	sz = size + sizeof(struct mem_cell);

#ifdef MINALLOC
	if (sz < MINALLOC)
		sz = MINALLOC;

#endif				/*  */
#ifdef VERBOSE
	{
		static mem arr[2];
		m_size(arr) = sz;
		noise("WANTED", arr);
	}
#endif				/*  */
	__alloca_alloc = malloc;	/* We'll be messing with the heap now TVM */

#ifdef LAZY_FREE
	ptr = __search_chunk(sz);
	if (ptr == 0) {

#endif				/*  */
		/* First deal with the freed list */
		if (__freed_list) {
			while (__freed_list) {
				ptr = __freed_list;
				__freed_list = m_next(__freed_list);
				if (m_size(ptr) == sz) {

					/* Oh! Well that's lucky ain't it :-) */
					noise("LUCKY MALLOC", ptr);
					return ptr + 1;
				}
				__insert_chunk(ptr);
			}
			ptr = m_next(chunk_list);
			if (ptr + m_size(ptr) >= (void *) sbrk(0)) {

				/* Time to free for real */
				m_next(chunk_list) = m_next(ptr);
				if (ptr == m_next(ptr))
					chunk_list = 0;
				free(ptr + 1);
			}
#ifdef LAZY_FREE
			ptr = __search_chunk(sz);

#endif				/*  */
		}
#ifndef LAZY_FREE
		ptr = __search_chunk(sz);

#endif				/*  */
		if (ptr == 0) {

#ifdef MCHUNK
			unsigned int alloc =
			    (MCHUNK * ((sz + MCHUNK - 1) / MCHUNK) - 1);
			if ((ptr = __mini_malloc(alloc)) != NULL)
				__insert_chunk(ptr - 1);

			else {	/* Oooo, near end of RAM */
				unsigned int needed = alloc;
				alloc /= 2;
				while (alloc > 256 && needed) {
					ptr = __mini_malloc(alloc);
					if (ptr) {
						if (alloc > needed)
							needed = 0;

						else
							needed -= alloc;
						__insert_chunk(ptr - 1);
					}

					else
						alloc /= 2;
				}
			}
			ptr = __search_chunk(sz);
			if (ptr == 0)
#endif				/*  */
			{

#ifndef MCHUNK
				ptr = __mini_malloc(size);

#endif				/*  */
#ifdef VERBOSE
				if (ptr == 0)
					noise("MALLOC FAIL", 0);

				else
					noise("MALLOC NOW", ptr - 1);

#endif				/*  */
				return ptr;
			}
		}
#ifdef LAZY_FREE
	}
#endif				/*  */
#ifdef VERBOSE
	ptr[1].size = 0x5555;

#endif				/*  */
	noise("MALLOC RET\n", ptr);
	return ptr + 1;
}


/* This function takes a pointer to a block of memory and inserts it into
 * the chain of memory chunks
 */
Static void __insert_chunk(mem * mem_chunk)
{
	register mem *p1, *p2;
	if (chunk_list == 0) {	/* Simple case first */
		m_next(mem_chunk) = chunk_list = mem_chunk;
		noise("FIRST CHUNK", mem_chunk);
		return;
	}
	p1 = mem_chunk;
	p2 = chunk_list;

	do {
		if (p1 > p2) {

			/* We're at the top of the chain, p1 is higher */
			if (m_next(p2) <= p2) {
				if (m_add(p2, m_size(p2)) == p1) {

					/* Good, stick 'em together */
					noise("INSERT CHUNK", mem_chunk);
					m_size(p2) += m_size(p1);
					noise("JOIN 1", p2);
				}

				else {
					m_next(p1) = m_next(p2);
					m_next(p2) = p1;
					noise("INSERT CHUNK", mem_chunk);
					noise("FROM", p2);
				}
				return;
			}
			if (m_next(p2) > p1) {

				/* In chain, p1 between p2 and next */
				m_next(p1) = m_next(p2);
				m_next(p2) = p1;
				noise("INSERT CHUNK", mem_chunk);
				noise("FROM", p2);

				/* Try to join above */
				if (m_add(p1, m_size(p1)) == m_next(p1)) {
					m_size(p1) += m_size(m_next(p1));
					m_next(p1) = m_next(m_next(p1));
					noise("JOIN 2", p1);
				}

				/* Try to join below */
				if (m_add(p2, m_size(p2)) == p1) {
					m_size(p2) += m_size(p1);
					m_next(p2) = m_next(p1);
					noise("JOIN 3", p2);
				}
				chunk_list = p2;	/* Make sure it's valid */
				return;
			}
		}

		else if (p1 < p2) {
			if (m_next(p2) <= p2 && p1 < m_next(p2)) {

				/* At top of chain, next is bottom of chain,
				 * p1 is below next
				 */
				m_next(p1) = m_next(p2);
				m_next(p2) = p1;
				noise("INSERT CHUNK", mem_chunk);
				noise("FROM", p2);
				chunk_list = p2;
				if (m_add(p1, m_size(p1)) == m_next(p1)) {
					if (p2 == m_next(p1))
						chunk_list = p1;
					m_size(p1) += m_size(m_next(p1));
					m_next(p1) = m_next(m_next(p1));
					noise("JOIN 4", p1);
				}
				return;
			}
		}
		chunk_list = p2;	/* Save for search */
		p2 = m_next(p2);
	} while (p2 != chunk_list);

	/* If we get here we have a problem, ignore it, maybe it'll go away */
	noise("DROPPED CHUNK", mem_chunk);
}


/* This function will search for a chunk in memory of at least 'mem_size'
 * when found, if the chunk is too big it'll be split, and pointer to the
 * chunk returned. If none is found NULL is returned.
 */
Static mem *__search_chunk(unsigned mem_size)
{
	register mem *p1, *p2;
	if (chunk_list == 0)	/* Simple case first */
		return 0;

	/* Search for a block >= the size we want */
	p1 = m_next(chunk_list);
	p2 = chunk_list;

	do {
		noise("CHECKED", p1);
		if (m_size(p1) >= mem_size)
			break;
		p2 = p1;
		p1 = m_next(p1);
	} while (p2 != chunk_list);

	/* None found, exit */
	if (m_size(p1) < mem_size)
		return 0;

	/* If it's exactly right remove it */
	if (m_size(p1) < mem_size + 2) {
		noise("FOUND RIGHT", p1);
		chunk_list = m_next(p2) = m_next(p1);
		if (chunk_list == p1)
			chunk_list = 0;
		return p1;
	}
	noise("SPLIT", p1);

	/* Otherwise split it */
	m_next(p2) = m_add(p1, mem_size);
	chunk_list = p2;
	p2 = m_next(p2);
	m_size(p2) = m_size(p1) - mem_size;
	m_next(p2) = m_next(p1);
	m_size(p1) = mem_size;
	if (chunk_list == p1)
		chunk_list = p2;

#ifdef VERBOSE
	p1[1].size = 0xAAAA;

#endif				/*  */
	noise("INSERT CHUNK", p2);
	noise("FOUND CHUNK", p1);
	noise("LIST IS", chunk_list);
	return p1;
}
