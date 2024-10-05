#include <kernel.h>
#include <printf.h>

/*
 *	This is inspired by Dale Schumacher's public domain dlibs, but rewritten
 *	from scratch. It's a very simple, very compact and surprisingly efficient
 *	malloc/free/memavail
 *
 *	These functions should *not* be used by core kernel code. They are
 *	used to support flat address space machines. They can be used by 32bit
 *	specific code and drivers but care should be taken to avoid fragmentation
 *	and leaks.
 */

#if defined(CONFIG_32BIT)

static uint32_t mfree;
static uint32_t mtotal;

#undef DEBUG_MEMORY

struct block
{
	struct block *next;
	size_t length; /* high bit set if used */
};

#define UNUSED(b) (!((b)->length & (1L<<31)))

static struct block start = { NULL, sizeof(struct block) };

/*
 * Add a memory block to the pool. Must be aligned to the alginment boundary
 * in use.
 */
void kmemaddblk(void *base, size_t size)
{
	struct block *b = &start;
	struct block *n = (struct block *) base;

	/* Run down the list until we find the last block. */

	while (b->next)
		b = b->next;

	/* Add the block. */

	b->next = n;
	n->next = NULL;
	n->length = size;
#if defined(DEBUG_MEMORY)
	kprintf("mem: add %p+%p\n", n, n->length);
#endif
	mfree += size;
	mtotal += size;
}

/*
 * Find the smallest unused block containing at least length bytes.
 */
static struct block *find_smallest(size_t length)
{
	static struct block dummy = { NULL, 0x7fffffff };
	struct block *smallest = &dummy;
	struct block *b = &start;

	while (b)
	{
		if (UNUSED(b)
			&& (b->length >= length)
			&& (b->length < smallest->length))
		{
			smallest = b;
		}

		b = b->next;
	}

	return (smallest == &dummy) ? NULL : smallest;
}

/*
 * Split the supplied block into a used section and an unused section
 * immediately following it (if big enough).
 */
static void split_block(struct block *b, size_t size)
{
	int32_t newsize = b->length - size; /* might be negative */
	if (newsize > sizeof(struct block) * 4)
	{
		struct block *n = (struct block *)((uint8_t *)b + size);
#if defined(DEBUG_MEMORY)
		kprintf("mem: split %p+%p -> ", b, b->length);
#endif
		n->next = b->next;
		b->next = n;
		b->length = size;
		n->length = newsize;
#if defined(DEBUG_MEMORY)
		kprintf("%p+%p and %p+%p\n", b, b->length, n, n->length);
#endif
	}

	b->length |= 0x80000000;
}

/*
 * Allocate a block.
 */
void *kmalloc(size_t size, uint8_t owner)
{
	struct block *b;

	used(owner);	/* For now */
	size = (size_t)ALIGNUP(size) + sizeof(struct block);
	b = find_smallest(size);
	if (!b)
		return NULL;

	split_block(b, size);
#if defined(DEBUG_MEMORY)
	kprintf("mem: alloc %p+%p\n", b, b->length);
	kprintf("malloc allocates %p\n", b + 1);
#endif
	mfree -= b->length;
	return b + 1;
}

/*
 * Merge all adjacent free blocks in the chain.
 */
static void merge_all_blocks(void)
{
	struct block *b = &start;

	while (b->next)
	{
		struct block *n = b->next;

		if (UNUSED(b)
			&& UNUSED(n)
			&& (((uint8_t*)b + b->length) == (uint8_t*)n))
		{
			/* Two mergeable blocks are adjacent. */
#if defined(DEBUG_MEMORY)
			kprintf("mem: merge %p+%p and %p+%p -> ",
				b, b->length, n, n->length);
#endif
			b->next = n->next;
			b->length += n->length;
#if defined(DEBUG_MEMORY)
			kprintf("%p+%p\n", b, b->length);
			kprintf("next now %p\n", b->next);
#endif
		}
		else
		{
			/* Only move on to the next block if we're unable to merge
			 * this one. */
			b = n;
		}
	}
}

/*
 * Free a block. If there's a block immediately after this one, merge it.
 * (This maintains ordering within split blocks.)
 */
void kfree(void *p)
{
	struct block *b = (struct block *)p - 1;

	if (p == NULL)
		return;
#if defined(DEBUG_MEMORY)
	kprintf("mem: free %p+%p\n", b, b->length);
	kprintf("malloc frees %p\n", p);
#endif

	if (UNUSED(b))
		panic(PANIC_BADFREE);
	
	b->length &= 0x7fffffff;
	mfree += b->length;
	merge_all_blocks();
}

void kfree_s(void *p, size_t unused)
{
	/* FIXME: we could do length checks ? */
	kfree(p);
}

unsigned long kmemavail(void)
{
	return mfree;
}

unsigned long kmemused(void)
{
	return mtotal - mfree;
}

#endif
