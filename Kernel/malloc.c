#include <kernel.h>
#include <printf.h>

/*
 *	From Dale Schumacher's public domain dlibs. It's a very simple, very
 *	compact and surprisingly efficient malloc/free/memavail
 *
 *	These functions should *not* be used by core kernel code. They are
 *	used to support flat address space machines. They can be used by 32bit
 *	specific code and drivers but care should be taken to avoid fragmentation
 *	and leaks.
 */

/*
 *	MAX_POOLS is the number of discontiguous memory pools we have.
 */

#if defined(CONFIG_32BIT)

#ifndef MAX_POOLS
#define	MAX_POOLS		1
#endif

#define	FREE		0x00
#define	USED		0x80
#define	NULL_BLOCK		0x80000000L
static char *_mblk[MAX_POOLS];	/* system memory heaps */
static long _msiz[MAX_POOLS];	/* allocated heap sizes */
static unsigned long memtotal;

/*
 *	Up to MAX_POOLS heaps are allocated as the operating system boots.
 *	These heaps are them divided into blocks for parcelling out by the
 *	user-callable memory allocation routines. Each heap beings with a
 *	pointer to the first free block, or NULL if there are no free blocks
 *	in this heap.  Each block begins with a 4-byte header which defines
 *	the number of bytes in the block, including the header.  Since blocks
 *	in a heap are known to be contiguous, this value also defines the
 *	beginning of the next block in the heap.  The high bit of the header
 *	is set if the block is used and clear if it is free.  The heaps ends
 *	with a block header which indicates a used block containing 0 bytes.
 *	The is the constant value NULL_BLOCK.  Free blocks contain an additional
 *	pointer field, immediatly following the header, which is a pointer to
 *	the header of the next free block, or NULL.
 */

/*
 *	Split block at *<addr> into a used block containing <size> bytes
 *	and a free block containing the remainder.
 */
static long *splitblk(register long **addr, long size)
{
	register long n, *p, *q;
	n = *(p = *addr);	/* get actual block size */
	if (n > (size + 8L)) {	/* is it worth splitting? */
		n -= size;

		/* calculate "break" point */
		q = ((long *) (((char *) p) + size));
		p[0] = size;
		q[0] = n;
		q[1] = p[1];
		*addr = q;
	}

	else			/* not worth splitting */
		*addr = ((long *) p[1]);	/* remove from free list */
	*((char *) p) = USED;	/* mark block "used" */
	return (p);
}


/*
 *	Find the smallest unused block containing at least <size> bytes.
 */
static long *findblk(register long size)
{
	register int i;
	register long n, tsiz = 0x7FFFFFFFL, **p, *q, *tptr = NULL;
	for (i = 0; i < MAX_POOLS; ++i) {
		if ((p = ((long **) _mblk[i])) == NULL)
			continue;	/* skip unavailable heaps */
		while ((q = *p) != NULL) {
			n = *q;
			if ((n >= size) && (n < tsiz)) {	/* it fits */
				tsiz = n;
				tptr = ((long *) p);
			}
			p = ((long **) (q + 1));
		}
	}
	return (tptr);
}


/*
 *	Merge adjacent "free" blocks in heap <i>.  Links in the free chain
 *	are guarenteed to be in forward order.
 */
static void mergeblk(int i)
{
	register long n, *p, *q;
	p = (long *) _mblk[i];
	if ((p = ((long *) *p)) == NULL)	/* empty chain */
		return;
	while ((q = ((long *) p[1])) != NULL) {
		n = *p;
		if (((char *) p) + n == ((char *) q)) {	/* adjacent free block */
			p[1] = q[1];	/* re-link free chain */
			*p += *q;	/* adjust block size */
		} else
			p = q;
	}
}


/*--------------------- Documented Functions ---------------------------*/
void *kmalloc(size_t size)
{
	register long *p;
	if (size <= 4L)
		size = 8L;	/* minimum allocation */
	else
		size = (size + 5L) & ~1L;	/* header & alignment */
	if ((p = findblk(size)) == NULL)
		return (NULL);
	/* FIXME: check cast */
	p = splitblk((long **) p, size);
	return (p + 4);		/* skip over header */
}

void *kzalloc(size_t size)
{
	void *p = kmalloc(size);
	if (p)
		memset(p, 0, size);
	return p;
}

void kfree(void *ptr)
{
	register long *addr = ptr;
	register int i;
	register long *p, *q;
	if (addr == NULL)
		return;
	--addr;			/* point to block header */
	for (i = 0; i < MAX_POOLS; ++i) {
		if ((p = ((long *) _mblk[i])) == NULL)
			continue;	/* skip unavailable blocks */
		if ((addr < p)
		    || (addr > ((long *) (((char *) p) + _msiz[i]))))
			continue;	/* block range check */
		while ((q = ((long *) *p)) != NULL) {
			++q;
			if ((addr < q) && (addr > p))
				break;
			p = q;
		}
		*((char *) addr) = FREE;	/* link into free chain */
		addr[1] = *p;
		*p = ((long) addr);
		mergeblk(i);
		return;
	}
	panic("bad free");
}


/* FIXME: We ought to keep this as a running total */
unsigned long kmemavail(void)
{
	register int i;
	register unsigned long n = 0L;
	register long **p, *q;
	for (i = 0; i < MAX_POOLS; ++i) {
		if ((p = ((long **) _mblk[i])) == NULL)
			continue;	/* skip unavailable heaps */
		while ((q = *p) != NULL) {
			n += *q;
			p = ((long **) (q + 1));
		}
	} return (n);
}

unsigned long kmemused(void)
{
	return memtotal - kmemavail();
}

/*
 *	Add a memory block to the pool
 */
void kmemaddblk(void *base, size_t size)
{
	register int i;
	register char *p;
	register long *q;
	for (i = 0; i < MAX_POOLS; ++i) {
		if (_mblk[i] != NULL)
			continue;	/* skip used heaps */
		_mblk[i] = base;
		_msiz[i] = size;
		q = ((long *) (p + size));	/* thread starting blocks */
		q[-1] = NULL_BLOCK;
		q = ((long *) (p + 4L));
		q[-1] = (long) q;
		q[0] = (size - 8L);
		q[1] = 0;
		p[4] = FREE;
		memtotal += size;
		return;
	}
	kputs("Too many memory pools\n");
}

#endif
