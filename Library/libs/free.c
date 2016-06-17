/*
 *	This is an ANSI C version of the classic K & R memory allocator. The only
 *	real difference here is that we handle large allocations and signs correctly
 *	which the original didn't do portably. Specifically we
 *	- correctly handle signed sbrk when the largest allocation allowed is
 *	  unsigned
 *	- catch the case of a malloc close to the full size_t overflowing in the
 *	  nblock computation.
 */

#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include "malloc.h"

void free(void *ptr)
{
	struct memh *mh = MH(ptr), *p;

	if (ptr == NULL)
		return;

	/* Find the free list block that is just before us */
	for (p = __mfreeptr; !(p < mh && mh < p->next); p = p->next)
		if (p >= p->next && (p < mh || mh < p->next))
			break;
	/* Fix up and if we can merge forward */
	if (mh + mh->size == p->next) {
		mh->size += p->next->size;
		mh->next = p->next->next;
	} else
		mh->next = p->next;
	/* Ditto backwards */
	if (p + p->size == mh) {
		p->size += mh->size;
		p->next = mh->next;
	} else {
		p->next = mh;
	}
	__mfreeptr = p;
}
