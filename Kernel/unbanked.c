/*
 *	Derived from:
 *	Memory allocator for ELKS. We keep a hole list so we can keep the
 *	malloc arena data in the kernel not scattered into hard to read 
 *	user memory.
 *
 *	Provide memory management for linear address spaces, either systems
 *	with base/limit type functionality (eg Z180) or just plain large address
 *	spaces with no useful bases at all (eg 68000)
 *
 *	On Z180 we use linear but we still have banking. On 68K binaries just have
 *	to be relocated and you can't swap.
 *
 *	For Z180 set this up with the memory range present. At the moment we only
 *	support one linear block but the hole manager can manage multiple non linear
 *	chunks if need be.
 *
 *	The proposed way to run Z180 is
 *
 *	User Space
 *	0-xxxx		Mapped to physical blocks holding user binary, vectors
 *			and some small stubs to switch bank
 *	xxxx-FFFF	Mapped to somewhere like ROM (stray write protection)
 *
 *	Kernel Space
 *	0-EFFF		Kernel image including other end of stubs
 *	F000		Udata of process (plus window space to make user
 *			copies easier)
 *
 *	(the upper being exchanged temporarily to access user memory, or we
 *	could put the copiers above F000 and switch the 0-EFFF bank but that
 *	makes irq handling uglier I suspect)
 *
 *	For 8086 we actually want two holes in some cases, one for cs: and one
 *	for ds: but we don't have all the vector hassle as we have a vaguely
 *	real notion of user/supervisor state.
 *
 *	Set:
 *	CONFIG_BANKED_LINEAR
 *	COMMON_BANKS	-	number of 256 byte pages of udata etc needed
 *				with each process
 *
 */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>

#ifdef CONFIG_BANK_LINEAR

/*
 *	Worst case is twice as many as allocations plus 1 I think ?
 */

#define MAX_SEGMENTS		((PTABSIZE*2) + 1)

struct hole {
	uint16_t base;		/* Pages */
	uint16_t extent;	/* Pages */
	struct hole *next;	/* Next in list memory order */
	__u8 flags;		/* So we know if it is free */
#define HOLE_USED		1
#define HOLE_FREE		2
#define HOLE_SPARE		3
#define HOLE_SWAPPED		8
};

static struct hole holes[MAX_SEGMENTS];

struct malloc_head {
	struct hole *holes;
	int size;
};

struct malloc_head memmap = { holes, MAX_SEGMENTS };

/*
 *	Find a spare hole.
 */

static struct hole *alloc_hole(struct malloc_head *mh)
{
	struct hole *m = holes;
	int ct = MAX_SEGMENTS;

	while (ct--) {
		if (m->flags == HOLE_SPARE)
			return m;
		m++;
	}
	return NULL;
}

/*
 *	Split a hole into two
 */

static int split_hole(struct malloc_head *mh, struct hole *m, uint16_t len)
{
	struct hole *n;
	uint16_t spare = m->extent - len;

	if (!spare)
		return 0
		    /*
		     *      Split into one allocated one free
		     */
		    n = alloc_hole();
	if (n == NULL)
		return ENOMEM;
	m->extent = len;
	n->base = m->base + len;
	n->extent = spare;
	n->next = m->next;
	m->next = n;
	n->flags = HOLE_FREE;
	return 0;
}

/*
 *	Merge adjacent free holes
 */

static void sweep_holes(struct malloc_head *mh)
{
	struct hole *m = mh->holes;

	while (m != NULL && m->next != NULL) {
		if (m->flags == HOLE_FREE && m->next->flags == HOLE_FREE &&
		    m->base + m->extent == m->next->base) {
			m->extent += m->next->extent;
			m->next->flags = HOLE_SPARE;
			m->next = m->next->next;
		} else
			m = m->next;
	}
}

#if 0

void dmem(struct malloc_head *mh)
{
	struct hole *m;
	char *status;
	if (mh)
		m = mh->holes;
	else
		m = memmap.holes;
	do {
		switch (m->flags) {
		case HOLE_SPARE:
			status = "SPARE";
			break;
		case HOLE_FREE:
			status = "FREE";
			break;
		case HOLE_USED:
			status = "USED";
			break;
		default:
			status = "DODGY";
			break;
		}
		printk("HOLE %x size %x next start %x is %s\n", m->base,
		       m->extent, m->base + m->extent, status);
		m = m->next;
	} while (m);
}

#endif

/*
 *	Find the nearest fitting hole
 */

static struct hole *best_fit_hole(struct malloc_head *mh, uint16_t size)
{
	struct hole *m = mh->holes;
	struct hole *best = NULL;

	while (m) {
		if (m->flags == HOLE_FREE && m->extent >= size)
			if (!best || best->extent > m->extent)
				best = m;
		m = m->next;
	}
	return best;
}

static struct hole *mm_alloc(uint16_t pages)
{
	/*
	 *      Which hole fits best ?
	 */
	struct hole *m;

	m = best_fit_hole(&memmap, pages);
	if (m == NULL)
		return NULL;

	/*
	 *      The hole is (probably) too big
	 */

	if (split_hole(&memmap, m, pages))
		return NULL;

	m->flags = HOLE_USED;
	m->refcount = 1;

	return m;
}

static int maps_needed(uint16_t size)
{
	uint16_t banks = (size + 255) >> 8 + COMMON_BANKS;
	/* 3 for uarea + common code copy */
	return banks;
}

/*
 * Try and swap stuff out if we can. If we run out of holes we will also swap
 * which should tidy up the space no end.
 */
static uint16_t pagemap_do_alloc(ptptr p, uint16_t size)
{
	uint16_t hole;
#ifdef SWAPDEV
	while ((hole = mm_alloc(maps_needed(size))) == 0) {
		if (swapneeded(p, 1) == NULL)
			return 0;
	}
#else
	hole = mm_alloc(maps_needed(size));
#endif
	return hole;
}

int pagemap_alloc(ptptr p)
{
	uint16_t hole = pagemap_do_alloc(p, udata.u_ptab->p_top);
	if (hole == 0)
		return ENOMEM;
	p->p_page = hole;
	return 0;
}

static int pagemap_can_alloc(uint16_t size)
{
	uint16_t hole = pagemap_do_alloc(p, size);
	if (hole) {
		mm_free(hole);
		return 1;
	}
	return 0;
}

/*
 *	Realloc our buffer. Naïve implementation.
 */
int pagemap_realloc(usize_t size)
{
	if (size == udata.u_ptab->p_top)
		return 0;
	if (size > udata.u_ptab->p_top && pagemap_can_alloc(size))
		return ENOMEM;
	pagemap_free(udata.u_page);
	pagemap_do_alloc(udata.u_ptab, size);
	/* Our vectors are below the base/limit ranges so don't get touched and
	   our uarea is above so that's fine too */
	/* Propogate the new allocation into our uarea */
	udata.u_page = udata.u_ptab->p_page;
	return 0;
}

/*
 *	Free a segment.
 */

void pagemap_free(ptptr p)
{
	struct hole *m = (struct hole *) p->page;
	if (m->flags != HOLE_USED)
		panic(PANIC_DOUBLE_FREE);
	m->flags = HOLE_FREE;
	sweep_holes(mh);
}

/*
 *	This is the only spot which cares what size a "block" is. For now its 256
 *	bytes because that makes shifting it really cheap!
 */
static usize_t pagemap_mem_used(void)
{
	struct hole *m;
	unsigned int ret = 0;

	while (m != NULL) {
		if (m->flags == HOLE_USED)
			ret += m->extent;
		m = m->next;
	}
	return ret << 2;
}

/*
 *	Initialise the memory manager. Pass the start and end in terms
 *	of blocks.
 */

void pagemap_setup(uint16_t start, uint16_t end)
{
	struct hole *holep = &holes[MAX_SEGMENTS - 1];

	/*
	 *      Mark pages free.
	 */
	do {
		holep->flags = HOLE_SPARE;
	} while (--holep > holes);

	/*
	 *      Single hole containing all user memory.
	 */
	holep->flags = HOLE_FREE;
	holep->base = start;
	holep->extent = end - start;
	holep->next = NULL;

}

#endif
