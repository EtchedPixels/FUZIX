/*
 *	A simple buddy allocator. Set BUDDY_SIZE to the number of blocks you
 *	may need to handle and BUDDY_NODESHIFT to the shift from block numbers
 *	to blocks. buddy_ram is a uint8_t pointer to the memory pool base.
 *
 *	All buddy data is kept outside of the allocator and the data does not
 *	therefore need to be power of two aligned (but can be).
 *
 *	The caller can use buddy_pin() to pin down blocks that are not available
 *	eg to align the allocator on powers of two boundaries
 *
 *	The main trick here for simplicity is to number the nodes from one not
 *	zero and to keep them together. That means you can move up/down by
 *	shifting the node number (left side on down) and you can find the
 *	pair by xoring the node number with 1.
 *
 *	We keep an owner because we need to support eviction of blocks for
 *	swapping.
 */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>

#ifdef CONFIG_BUDDY_MALLOC

static uint8_t buddy[2 * BUDDY_SIZE];	/* Node 0 is not used */
static uint16_t inuse, total;

#define NODE_FREE	0xFF
#define NODE_UNAVAIL	0x00

/* Pin a bottom level node */
void buddy_pin(uint16_t node, uint8_t owner)
{
	/* Optional debug - check first entry is NODE_FREE */
	do {
		buddy[node] = owner;
		node /= 2;
	} while(node && buddy[node] == NODE_FREE);
}

/* Unpin a bottom level node */
static void buddy_unpin(uint16_t node)
{
	uint16_t o;

	/* Our node becomes free */
	buddy[node] = NODE_FREE;
	/* Walk to the top of the tree freeing the parent if the other
	   child is also free. As we go remember the pair owner */
	while(node > 1 && (o = buddy[node^1]) == NODE_FREE)
		buddy[node /= 2] = NODE_FREE;
	/* If we stopped part way then set the owner of the nodes above us
	   to the pair so that we propogate useful ownership info up.
	   Optional */
	while(node > 1)
		buddy[node /= 2] = o;
}

/* Free a block and propogate both free and ownership information updates */
static uint8_t buddy_free(uint16_t node, uint8_t sizeshift)
{	
	uint16_t n = 1 << sizeshift;	/* How many blocks to free */
	uint16_t i;

	node <<= depth;		/* Turn our node into the leftmost base node of
				   the level we need */
	/* Unpin all our blocks from the bottom up */
	for (i = 0; i < n; i++)
		buddy_upin(node++);
	inuse -= n;
}

/* Allocate a block using the buddy allocator if space is available */
static int16_t buddy_alloc(uint8_t sizeshift, uint8_t owner)
{
	/* There are n slots at offset n: isn't that neat */

	uint16_t ct = n;
	uint16_t node = n;
	uint16_t n = BUDDY_SIZE >> sizeshift;

	while(ct--) {
		if (buddy[node] == NODE_FREE) {
			/* Found a free node at our level. Turn it to base
			 * */
			node <<= sizeshift;
			for (ct = 0; ct < n; ct++)
				pin(node + ct, owner);
			inuse += n;
			return node;
		}
		node++;
	}
	return -1;
}

#if 0 // for swap
uint8_t buddy_evict(uint16_t base, uint8_t sizeshift)
{
	uint16_t node = base;
	uint8_t n = BUDDY_SIZE >> sizeshift;
	for (ct = 0; ct < n; ct++)
		if (!can_evict(node++))
			return EBUSY;
	}
	node = base;
	for (ct = 0; ct < n; ct++)
		evict(node);
		buddy_unpin(node++);
	}
}
#endif

/* We work entirely in nodes but the conversions are simple */

static uint8_t *buddy_node2addr(uint16_t node)
{
	return buddy_ram + node << BUDDY_NODESHIFT;
}

static uint8_t buddy_addr2node(uint16_t *p)
{
	size_t n = (p - buddy_ram) >> BUDDY_NODESHIFT;
}

/* There are faster ways to do this using ffs on some processors or and
 * masking to binary search.
 *
 * Note: gives nonsense result for a 0 size request
 */
static uint8_t buddy_sizetoshift(uint32_t n)
{
	uint8_t ct = 0;

	n--;
	n >>= BUDDY_NODESHIFT;
	while(n) {
		n >>= 1;
		ct++;
	}
	return ct;
}

void buddy_init(uint16_t start, uint16_t end)
{
	/* It starts with everything owned by 0 */
	while (start < end) {
		total++;
		buddy_upin(start);
	}
	inuse = 0;
}

/* Supply the standard memory allocator layer so that we can plug this into the
   banking functions just like malloc.c */

unsigned long kmemavail(void)
{
	return (total - inuse) << BUDDY_NODESHIFT;
}

unsigned long kmemused(void)
{
	return inuse << BUDDY_NODESHIFT;
}

void kfree_s(void *p, size_t size)
{
	buddy_free(buddy_addr2node(p), buddy_sizetoshift(size));
}

void *kmalloc(size_t size, uint8_t owner)
{
	/* We will need to pass owners around eventually to both allocators */
	int16_t node = buddy_alloc(buddy_sizetoshift(size), owner);
	if (node < 0)
		return NULL;
	return buddy_node2addr(p);
}

#if 0
/* For now use default flat one */
usize_t valaddr(const char *pp, usize_t lw)
{
	uint16_t node = buddy_addr2node(pp);
	uint16_t base = pp - buddy_node2addr(node);
	int32_t l = lw;

	while(l > 0) {
		if (buddy[node] != udata.u_page) {
			if (lw == l)
				udata.u_error = EFAULT;
			return lw - l;
		l -= (1 << BUDDY_NODESHIFT) - base;
		base = 0;
	}
	return lw;
}

#endif
#endif

