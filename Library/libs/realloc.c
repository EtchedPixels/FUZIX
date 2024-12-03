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
#include <string.h>
#include "libmalloc.h"

/*
 * We cannot just free/malloc because there is a pathalogical case when we free
 * a block which is merged with the block before and then we allocate some of the
 * combined block. In that case we will a new header into the middle of the user
 * bytes.
 */
void *realloc(void *ptr, size_t size)
{
	struct memh *mh = MH(ptr);
	void *np;
	size_t nblocks;

	if (size == 0) {
		free(ptr);
		return NULL;
	}

	if (ptr == NULL)
		return malloc(size);

	nblocks =
	    (size + sizeof(struct memh) + sizeof(struct memh) -
	     1) / sizeof(struct memh);

	/* If size in mem blocks is sufficiently similar (make this fuzzier ?) */
	if (nblocks == mh->size)
		return ptr;

	if (nblocks < mh->size)
		return ptr;
	np = malloc(size);
	if (np == NULL)
		return ptr;
	memcpy(np, ptr, mh->size * sizeof(struct memh));
	free(ptr);
	return np;
}
