/*
 *	Trivial left packing buddy allocator favouring memory efficiency
 *	Intended for 68K systems using the simple MMU concept. The only
 *	real oddities here are that we left pack to try and keep big
 *	chunks free and that we know the owner of any leaf block which
 *	allows our swapper to evict the right people during a brk() grow or
 *	in theory (with compiler hooks added) on a stack extend.
 *
 *	BUDDY_NUMLEVEL:	levels of allocator
 *	BUDDY_BLOCKBITS: bits in the block size (eg 12 for 4K)
 *	BUDDY_BASE: base address of the managed block
 *	BUDDY_START: first usable address in managed range
 *	BUDDY_TOP: top address of usable memory for buddy
 *	BUDDY_TABLE: the buddy table, usually set to BUDDY_START to take it
 *		     from the memory pool.
 *	BUDDY_TREESIZE: size of allocation for the buddy tree
 *
 *	buddy_level: offsets for each level in tree
 *
 *	These can all be defined to variables if needed
 *
 *	The smallest size is level 0, that makes things simpler
 *	for the rest of the logic.
 *
 *	The simple MMU needs BUDDY_BASE to be power of two aligned, but
 *	not BUDDY_START (although that is preferable). The code itself
 *	just needs CPU appropriate alignment.
 *
 *	The simple MMU works as follows. All user space addresses are
 *	translated according to the rule
 *
 *	pa = (va & mask_reg) ^ base_reg;
 *	fault = (va & ~mask_reg) != 0 && (va & ~mask_reg) != ~mask_reg;
 *
 *	Processes are set up with stack at va FFFFFFFF and base at 00000000
 *	on a suitably power of 2 aligned address allocated from the buddy
 *	for the size needed. Mask is set to size - 1 and base is set to the
 *	base of the block. When extending a larger block is allocated
 *	(possibly extended), and the code is copied to the bottom of the new
 *	block if we extended down, and the stack to the new top if we extended
 *	up. The base and mask are then updated so the process doesn't see the
 *	fact it's been physically stretched.
 *
 *	The table consumes an effective 2 bytes per block. So for 8MB of
 *	RAM in 2K blocks we need one block. In practise for most machines
 *	we can simply assume we lose one block and set the tables accordingly
 *	rather than dynamically. Given 2K is actually undersized for blocks
 *	on most systems this is doubly true.
 *
 *	NOT YET DEBUGGED DO NOT USE!!!!
 */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <buddy.h>

#ifdef CONFIG_BUDDY_MMU

#define BUDDY_FREE	0xFF

extern uint16_t buddy_level[BUDDY_NUMLEVEL];

/* Ok this is crap FIXME but will do for now */
static int buddy_get_level(usize_t size)
{
	int l = 0;
	size >>= BUDDY_BLOCKBITS;
	while(size) {
		size >>= 1;
		l++;
	}
	return l;
}

/* Turn a block number at a given level into a memory address for the
   map bit and a bit number */
static uint8_t *map_buddy(uint32_t n, int level)
{
	n >>= level + BUDDY_BLOCKBITS;	/* Index into level */
	/* Add in the precomputed tree level base */
	return BUDDY_TABLE + buddy_level[level] + n;
}

/* Mark the required bytes as 0xFF (free) or n (busy). Handle the propogation
   of busyness into the higher tree levels */
static void buddy_markv(uint32_t n, int level, uint8_t owner)
{
	uint8_t *p;
	do {
		p = map_buddy(n, level);
		/* FIXME: check for freeing free, owning owned */
		*p = owner;

		/* If we are freeing then a non free partner means our parent remains busy,
		   if we are allocating then it means our parent is already busy. Either
		   way we can stop !

		   This looks so much better in B than C, but the compiler will generate
		   nice asm for it anyhow */
		p = (uint8_t *) (((size_t) p) ^ 1);
		/* FIXME: This logic below is wrong for free/free merging */
	} while (level++ < BUDDY_NUMLEVEL && *p != BUDDY_FREE);
	/* All free or all full */
}

void buddy_free(uint8_t * p, int level)
{
	uint32_t n = p - BUDDY_BASE;
	buddy_markv(n, level, BUDDY_FREE);
}

/* Turn a map entry into an offset relative to the buddy base */
static uint32_t map_to_ptr(int i, int level)
{
	return BUDDY_BLOCKSIZE << level;
}

/* Could be optimised a bit by walking down the tree levels rather than
   scanning across ? However remember that in most of our use cases the
   largest allocation is going to be a small number of 4K or 8K pages */
uint8_t *buddy_alloc(int level, uint8_t owner)
{
	unsigned int i;
	uint32_t n;

	/* The level we will scan for a free byte */
	uint16_t level_size = 1 << (BUDDY_NUMLEVEL - level);
	uint8_t *p = BUDDY_TABLE + buddy_level[level];

	for (i = 0; i < level_size; i++) {
		if (*p++ == BUDDY_FREE) {
			n = map_to_ptr(i, level);
			level_size = 1 << level;
			/* Mark ourself busy at all the levels */
			for (i = 0; i < level_size; i++) {
				buddy_markv(n, 0, owner);
				n += BUDDY_BLOCKSIZE;
			}
			return n + BUDDY_BASE;
		}
	}
	/* Out of memory */
	return NULL;
}

/*
 *	Call the evictor with the owner id of each occupied block
 *	that is needed to make the pair of our buddy block free.
 */
int buddy_evict(uint8_t *p, int level, int (*evict) (uint8_t owner))
{
	/* Find the map for our partner at this level, and free it up */
	uint8_t *m = map_buddy((p - BUDDY_BASE) ^
			       (BUDDY_BLOCKSIZE << level), 0);
	int i = 1 << level;
	int r;
	uint8_t last = BUDDY_FREE;
	uint8_t d;

	while (i--) {
		d = *m++;
		if (d != BUDDY_FREE) {
			if (d != last && (r = evict(d)))
				return r;
			last = d;
		}
	}

	return 0;
}

/* Given a pointer return the base address of the block at this level. Our
   memory management layer needs this so that it can work out where the
   new bigger MMU base is relative to the current allocation when we evict
   the pair with buddy_evict */
uint8_t *buddy_base(uint8_t * p, int level)
{
	size_t mask = (BUDDY_BLOCKSIZE << level) - 1;
	/* Do the maths so we don't assume power of two alignment of base. With
	   luck gcc will figure out the optimisation for the simple case 8) */
	size_t n = p - BUDDY_BASE;
	return BUDDY_BASE + (n & ~mask);
}

/* Given a size return the needed order */
uint8_t buddy_order(size_t s)
{
	uint8_t level = 0;
	/* This all comes out as constant maths on s */
	s = (s + BUDDY_BLOCKSIZE - 1) >> BUDDY_BLOCKBITS;
	/* Simple bit count - we could optimise if needed ! */
	while (s >>= 1)
		level++;
	if (level >= BUDDY_NUMLEVEL)
		return 0xFF;
	return level;
}

/* -- Can go in discard along with any platform computation code for
      doing dynamic setup -- */

void buddy_init(void)
{
	uint8_t *p = BUDDY_START;

	if (BUDDY_START == BUDDY_TABLE)
		p += BUDDY_TREESIZE;

	/* Mark every block as currently in use, and minimum size. Must be picked
	   so that the mapping layers fall on byte boundaries */
	memset(BUDDY_START, 0x00, BUDDY_TREESIZE);

	/* Free all the blocks that are truely free - saves us
	   having code to set it all up nicely */
	while (p < BUDDY_TOP) {
		buddy_free(p, 0);
		p += BUDDY_BLOCKSIZE;
	}
}


/******************* MMU Logic *************************/
int do_pagemap_alloc(ptptr p, usize_t size)
{
	return 0;
	/* FIXME */
	uint8_t *m;
	uint8_t pn = p - ptab;
	uint8_t level = buddy_order(size);
	if (level == 0xFF)
		return ENOMEM;
	while (1) {
		m = buddy_alloc(level, pn);
		if (m) {
			/* Indicate the actual size handed back as we must
			   put the stack at the top */
			membase[pn] = m;
			p->p_top = (BUDDY_BLOCKSIZE << level) - 1;
			p->p_page = level;
			program_mmu(m, p->p_top);
			return 0;
		}
/* FIXME
		if (swapneeded(p, 1) == NULL)
			return ENOMEM; */
	}
}

int pagemap_alloc(ptptr p)
{
	return do_pagemap_alloc(p, p->p_top);
}

/* Reallocate for an exec or brk */
int pagemap_realloc(usize_t size)
{
	uint8_t pn = udata.u_ptab - ptab;
	/* We can do this better - on a shrink we can free some of our buddy
	   pages */
	/* FIXME: check if space before we do these .. */
	buddy_free(membase[pn], buddy_get_level(size));
	return do_pagemap_alloc(udata.u_ptab, size);
}

void pagemap_free(ptptr p)
{
	/* FIXME TODO */
}

unsigned long pagemap_mem_used(void)
{
	/* TODO */
	return 0;
}

static uint8_t grow_pn;

/* Kick out everyone unlucky enough to be in the way */
static int pagemap_evict(uint8_t owner)
{
	if (owner != grow_pn)
		return swapout(ptab + owner);
	return 0;
}

int pagemap_grow(usize_t size)
{
	uint8_t level = buddy_get_level(size);
	uint8_t *m, *n, *me, *ne;
	ptptr p;

	if (level == 0xFF)
		return ENOMEM;
	/* We don't give back for the moment */
	if (level <= udata.u_ptab->p_page)
		return 0;

	p = udata.u_ptab;
	grow_pn = p - ptab;

	m = membase[grow_pn];
	n = buddy_alloc(level, grow_pn);
	if (n)
		buddy_free(m, p->p_page);
	else {
		/* Have to do an eviction */
		/* Get the base of the bigger block we will expand into */
		n = buddy_base(m, level);
		/* Make room */
		if (buddy_evict(n, level, pagemap_evict))
			return ENOMEM;
		/* FIXME: do we need to check enough room before we do this ! */
		buddy_free(m, p->p_page);
		n = buddy_alloc(level, grow_pn);
		if (n == NULL)
			panic("insfram");
	}
	membase[grow_pn] = n;
	p->p_top = (1 << level) - 1;
	/* Now copy the code/data/bss/break area if it moved */
	if (m != n)
		memcpy(n, m, udata.u_break);
	/* Point to the byte after the end of memory */
	me = m + (BUDDY_BLOCKSIZE << p->p_page);
	ne = n + (BUDDY_BLOCKSIZE << level);
	/* The stack pointer is FFF.... so effectively negative, and we can
	   do our maths from the tail by using this */
	/* SECURITY FIXME: someone somewhere between entry and here needs to
	   spot out of range user stack pointers and kill the process */
	/* Copy the stack if it moved - need to copy a bit more FIXME */
	if (me != ne)
		memcpy(ne + udata.u_syscall_sp, me + udata.u_syscall_sp, -udata.u_syscall_sp);
	/* Zero the hole in the middle */
	memzero(n + udata.u_break,
		(1 << level) - udata.u_break + udata.u_syscall_sp);
	p->p_page = level;
	program_mmu(m, p->p_top);
	return 0;
}



/* Swappers to do yet  - simple swapmap stuff won't work - we need an 
   actual allocator for swap - should have a general purpose flat physical
   swap for bigger boxes */

void swapin(ptptr p, uint16_t map)
{
}

int swapout(ptptr p)
{
	return ENOMEM;
}

/* FIXME: THESE NEED WRITING PROPERLY FOR THE MMU */
usize_t _uget(const uint8_t *user, uint8_t *dest, usize_t count)
{
//	uint8_t tmp;
//	while(count--) {
//		tmp = *user++;
//		*dest++ = tmp;
//	}
	return 0;
}

int16_t _ugetc(const uint8_t *user)
{
	uint8_t tmp;
//	tmp = *user;
	return tmp;
}

uint16_t _ugetw(const uint16_t *user)
{
	uint16_t tmp;
//	tmp = *user;
	return tmp;
}

uint32_t ugetl(void *user, int *err)
{
	uint32_t tmp;
//	tmp = *(uint32_t *)user;
	return tmp;
}

int _ugets(const uint8_t *user, uint8_t *dest, usize_t count)
{
//	uint8_t tmp;
//	while(count--) {
//		tmp = *user++;
//		*dest++ = tmp;
//		if (tmp == '\0')
//			return 0;
//	}
//	/* Ensure terminated */
//	dest[-1] = '\0';
	return -1;
}

int _uput(const uint8_t *source, uint8_t *user, usize_t count)
{
//	uint8_t tmp;
//	while(count--) {
//		tmp = *source++;
//		*user++ = tmp;
//	}
	return 0;

}

int _uputc(uint16_t value,  uint8_t *user)
{
//	*user = value;
	return 0;
}

int _uputw(uint16_t value,  uint16_t *user)
{
//	*user = value;
	return 0;
}

int uputl(uint32_t value,  void *user)
{
//	*(uint32_t *)user = value;
	return 0;
}

int _uzero(uint8_t *user, usize_t count)
{
//	while(count--)
//		*user++=0;
	return 0;
}

arg_t _memalloc(void)
{
	udata.u_error = ENOSYS;
	return -1;
}

arg_t _memfree(void)
{
	udata.u_error = ENOSYS;
	return -1;
}

#endif
