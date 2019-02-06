/*
 *	Draft code only
 *
 *	This module manages a classic IBM PC system with up to 640K of RAM
 *	by splitting the memory into 4K 'pages'. Data and stack are allocated
 *	by finding a pair of chunks such that the the stack starts at FFFF
 *	and the data begins at 0000 but the 'hole' in the middle may be
 *	something else. If need be we re-arrange memory to cope with collisions.
 *	This allows us to avoid minix and ELKS chmem like messiness.
 *
 *	The kernel is assumed to be loaded in the lowest pages.
 *
 *	Set CONFIG_BANK_8086
 *	Call pagemap_init passing it the page above the kernel, and the total
 *	number of pages.
 *
 *	TODO
 *	- Review, Test, Debug
 *	- Finish swap support
 *	- Custom usermem including valaddr to handle holes.
 *	- Offset the page base to be a 512 byte boundary above the
 *	  end of the code (covering discard). Right now we can lose memory
 *	  because of EBDA (top 1K .. costs us 4K if present) and alignment
 *	  of end of code.
 *
 *	This module relies upon the split I/D execve code as well as the
 *	platform specific copy_page and zero_page functionality that is
 *	used to move far blocks around including overlapping ones.
 *
 *	For 286 we'll need to make page_t bigger and allow for marking
 *	of 'dead' ranges in the mapping (eg 640K to 1MB). The algorithm
 *	itself is fine with this. With 4K pages a 16MB 286 would have 4096
 *	pages. It's not clear the hole counting algorithm scales nicely this
 *	far but then it's not clear Fuzix does either 8) as you'd need at
 *	least 128 things running to fill the space given we have no plan
 *	to support all the crazy large model stuff that Xenix 286 could.
 *
 *	If you have an EMM implementation then it must implement the following
 *	routines
 *
 *	emm_alloc_bank(m)
 *	-	if a 64K EMM bank is not free set m->emm = 0 return ENOMEM
 *	-	otherwise set m->emm to your own non zero token and
 *		set m->dseg to the matching EMM page frame
 *
 *	emm_free_bamk(m)
 *	-	return a 64K EMM bank m->emm to to the pool
 *
 *	emm_select(emm)
 *	-	select the EMM bank matching the token m->emm
 *
 *	emm_space_used(void)
 *	-	return the number of KB of emm in use
 *	-	on EMM init also add the emm space to ramsize
 */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>

#ifdef CONFIG_BANK_8086

/* private : asm helpers */
extern void do_copy_page(uint16_t, uint16_t);
extern void do_zero_page(uint16_t);

typedef uint8_t page_t;

/* Assuming a 640K PC/XT. This model needs some work to handle a 16MB 286
   and it's not clear how to extend it to EMM */

/* 4K keeps the page count down into 8bits and also ensures the shift between
   page and segment is trivial */
#define PAGESIZE	4096
#define PAGESHIFT	12
#define PAGES_64K	(page_t)(65536L >> PAGESHIFT)
#define NUM_PAGES	((640) >> (PAGESHIFT -  10))

#define PAGES_IN(x)	(((x) + PAGESIZE - 1) >> PAGESHIFT)

/* 4K is a bit wasteful perhaps but it does mean that we have < 256 pages
   which makes life roll along much more nicely */
static page_t pagemap[NUM_PAGES];
static uint16_t total_pages;
static uint16_t free_pages;

/* Pointed to by udata.u_page2 */
static struct proc_map proc_mmu[PTABSIZE];

static uint16_t page_to_segment(page_t page)
{
	return (page << (PAGESHIFT - 4));
}

uint16_t get_data_segment(uint16_t pv)
{
	struct proc_map *m = (struct proc_map *)pv;
	return page_to_segment(m->dseg);
}

uint16_t get_code_segment(uint16_t pv)
{
	struct proc_map *m = (struct proc_map *)pv;
	return page_to_segment(m->cseg);
}

/* Mark a region as owned by us: FIXME once debugged #define this */
static void claim_region(page_t base, uint8_t len, uint8_t owner)
{
	memset(pagemap + base, owner, len);
	if (owner)
		free_pages -= len;
	else
		free_pages += len;
}

/* Claim the regions owned by a process, also used to free them */
static void claim_regions(struct proc_map *m, uint8_t owner)
{
	claim_region(m->cbase, m->csize, owner);
	claim_region(m->dbase, m->dsize, owner);
	claim_region(m->dbase + PAGES_64K -  m->ssize, m->ssize, owner);
}

static void claim_data_regions(struct proc_map *m, uint8_t owner)
{
	claim_region(m->dbase, m->dsize, owner);
	claim_region(m->dbase + PAGES_64K -  m->ssize, m->ssize, owner);
}

/* Size a free block by counting zero (free) bytes */
static uint8_t need_zeros(uint8_t *addr, uint8_t n)
{
	uint8_t ct = 0;
	while(*addr == 0 && ct++ < n);
	return ct;
}

static uint8_t *skip_used(uint8_t *p)
{
	while(*p && p < &pagemap[NUM_PAGES])
		p++;
	return p;
}

/* Finds the space but doesn't book it out - so we can look for slack and if
   not there try again

   Q: would a need zeros or self be useful so we could look for a match
   that works if we move our existing allocation rather than just do a new
   one ? */

static page_t alloc_data(uint8_t dataneed, uint8_t stackneed)
{
	uint8_t *addr = pagemap;
	uint8_t x;
	uint8_t y;

	while(addr = skip_used(addr)) {
		x = need_zeros(addr, dataneed);
		y = need_zeros(addr + PAGES_64K - stackneed, stackneed);
		if (x == dataneed) {
			if (y == stackneed)
				return addr - pagemap;
			else {
				addr++;
				continue;
			}
		} else {
			addr += x;	/* skip the whole space */
		}
	}
	return  0;
}

/* Do the same job for a code allocation - here we don't have to worry about
   space for the code and kernel copies (in fact we set CS: pointing offset
   into the space in order to keep the udata copy here. This is problematic
   if we implement shared code, so will require a cunning plan (tm) */

static page_t alloc_code(uint8_t need)
{
	uint8_t *addr = pagemap;
	uint8_t x;

	while(addr = skip_used(addr)) {
		x = need_zeros(addr, need);
		if (x == need)
			return addr - pagemap;
		addr += x;
	}
	return 0;
}

/* We don't yet deal with code segment sharing.. */

/*
 *	Helper to find a working memory map for a process. This doesn't
 *	clear or claim any memory merely set up the proc_mmu structure.
 *
 *	It's important it doesn't do anything non-reversible because we
 *	want to call this in situations where we save the old mmu status,
 *	mark the pages as free, try a new allocation and then if that fails
 *	reclaim the old one.
 */
static int do_process_create(uint8_t csize, uint8_t dsize, uint8_t ssize)
{
	struct proc_map *m = (struct proc_map *)udata.u_page2;
#ifdef CONFIG_IBMPC_EMM
	if (emm_alloc_bank(m) == 0) {
		m->dbase = alloc_data(m->dsize, m->ssize);
		if (m->dbase == 0)
			return -1;
		m->dseg = page_to_segment(m->dbase);
	}
#endif	
	m->cbase = alloc_code(csize);
	if (m->cbase == 0) {
#ifdef COFNIG_IBMPC_EMM
		emm_free_bank(m);
#endif		
		return -1;
	}
	m->csize = csize;
	m->dsize = dsize;
	m->ssize = ssize;
	m->cseg = page_to_segment(m->cbase);
	return 0;
}

static void copy_pages(page_t to, page_t from, uint16_t len)
{
	uint16_t ts = page_to_segment(to);
	uint16_t fs = page_to_segment(from);
	while(len--)
		do_copy_page(ts++, fs++);
}

static void zero_pages(page_t to, uint16_t len)
{
	uint16_t ts = page_to_segment(to);
	while(len--)
		do_zero_page(ts++);
}

/*
 *	A brk() or stack grow has collided with some other piece of memory.
 *	Try and find a place to move our data and stacks so that they can
 *	grow as desired. If need be start swapping stuff out.
 *
 *	Invariant: never called for emm banked processes
 */
static int relocate_data(uint8_t dsize, uint8_t ssize)
{
	struct proc_map *m = (struct proc_map *)udata.u_page2;
	page_t dbase;

	/* Mark our memory free */
	claim_data_regions(m, 0);
	
	/* If need be swap other processes out until we can find a suitable
	   hole pair. We aren't very bright here but it's not clear that being
	   smart helps anyway */
	while((dbase = alloc_data(dsize, ssize)) == 0) {
		if (swapneeded(udata.u_ptab, 1) == NULL)
			return ENOMEM;
	}
	if (dbase == 0)
		return ENOMEM;
	/* Move the segments. This needs care because we might have put
	   one of the two over the other */
	if (dbase < m->dbase) {
		/* Moved down - so we know that the new data is not over the old
		   stack, but the new stack may be over the old data */
		copy_pages(dbase, m->dbase, m->dsize);
		copy_pages(dbase + PAGES_64K - m->ssize,
			   m->dbase + PAGES_64K - m->ssize, m->ssize);
	} else {
		/* Moved up - so move the stack first */
		copy_pages(dbase + PAGES_64K - m->ssize,
			   m->dbase + PAGES_64K - m->ssize, m->ssize);
		copy_pages(dbase, m->dbase, m->dsize);
	}
	m->dbase = dbase;
	m->dsize = dsize;
	m->ssize = ssize;
	/* Mark the regions we used */
	claim_data_regions(m, udata.u_page);
	m->dseg = page_to_segment(m->dbase);
}

/* Stack growth via software handled fake fault */
static int stack_fault(uint8_t ssize)
{
	struct proc_map *m = (struct proc_map *)udata.u_page2;
	uint8_t *p = &pagemap[m->dbase + PAGES_64K - ssize];
	int8_t mod = ssize - m->ssize;
	
	/* Collided ? if so we will wrap */
	if (m->ssize + m->dsize > m->ssize)
		return ENOMEM;
#ifdef CONFIG_IBMPC_EMM
	if (m->emm) {
		m->ssize = ssize;
		return 0;
	}
#endif
	/* FIXME: check rlimitt */
	if (need_zeros(p, mod) != mod)
		return relocate_data(m->dsize, ssize);
	m->ssize += mod;
	claim_region(m->dbase + PAGES_64K - m->ssize, mod, udata.u_page);
	zero_pages(m->dbase + PAGES_64K - m->ssize, mod);
	return 0;
}

/* Data change via brk() */
static int data_change(uint8_t dsize)
{
	struct proc_map *m = (struct proc_map *)udata.u_page2;
	uint8_t *p = &pagemap[m->dbase + m->dsize];
	int8_t mod = dsize - m->dsize;

	if (dsize == m->dsize)
		return 0;

#ifdef CONFIG_IBMPC_EMM
	if (m->emm) {
		if (dsize > m->dsize && dsize + m->ssize < dsize)
			return ENOMEM;
		m->dsize = dsize;
		return 0;
	}
#endif	
	/* Give back pages */
	if (dsize < m->dsize) {
		claim_region(m->dbase + dsize, -mod, 0);
		m->dsize = dsize;
		return 0;
	}
	/* Growing and wraps ? */
	if (dsize + m->ssize > m->ssize)
		return ENOMEM;
	/* Claim pages */
	if (need_zeros(p, mod) != mod)
		return relocate_data(dsize, m->ssize);
	/* The caller with zero it */
	claim_region(m->dbase + m->dsize, mod, udata.u_page);
	m->dsize = dsize;
	return 0;
}


/* Called from execve when we exec a process. Kill off any old map and
   produce a new one. If we fail keep the old map and execve can return
   into it with an error. As we are a split I/D system with stacks this
   call is different to the simple 8bit one */
int pagemap_realloc(usize_t csize, usize_t dsize, usize_t ssize)
{
	struct proc_map *m = (struct proc_map *)udata.u_page2;
	static struct proc_map tmp;
	int err = 0;

	csize = PAGES_IN(csize);
	dsize = PAGES_IN(dsize);
	ssize = PAGES_IN(ssize);

	memcpy(&tmp, m, sizeof(tmp));
	claim_regions(m, 0);	/* Free our regions */
	if (do_process_create(csize, dsize, ssize)) {
		memcpy(m, &tmp, sizeof(tmp));
		err = ENOMEM;
	}
	claim_regions(m, udata.u_page);
	return err;
}

/* Report in Kbytes how much memory is in use */
usize_t pagemap_mem_used(void)
{
	usize_t r = (total_pages - free_pages) << (PAGESHIFT - 10);
#ifdef CONFIG_IBMPC_EMM
	r += emm_space_used();
#endif
	return r;
}

/* ktop is a page number as is ramtop */
void pagemap_init_range(page_t ktop, page_t ramtop)
{
	/* FIXME: should round _up_ on ktop! */
	ktop >>= (PAGESHIFT - 10);
	ramtop >>= (PAGESHIFT - 10);
	/* Reserve the kernel */
	memset(pagemap, 0xFE, ktop);
	memset(pagemap + ktop, 0, ramtop - ktop);
	/* Reserve any memory below 640K if we are say a 512K machine */
	memset(pagemap + ramtop, 0xFE, ramtop - NUM_PAGES);
	total_pages = ramtop - ktop;
	free_pages = total_pages;
}

/* Allocate the memory map for process p while ensuring process keep is not
   swapped out. In some cases the two are the same but when forking we can't
   swapout the new process anyway, but must avoid swapping out the live task */
static int do_pagemap_alloc(ptptr p, ptptr keep)
{
	struct proc_map *m = (struct proc_map *)udata.u_page2;
	/* This is called during a fork where we need to allocate a new
	   set of maps. At the point of calling p points to the new process
	   and udata the current one */

	/* Set up our new mmu pointers */
	p->p_page = p - ptab;
	p->p_page2 = (uint16_t)&proc_mmu[p->p_page];

	/* Create the new mmu allocation */
	while (do_process_create(m->csize, m->dsize, m->ssize))
		if (swapneeded(keep, 1) == NULL)
			return ENOMEM;

	/* Now claim our mapping */
	claim_regions(m, p->p_page);
	return 0;
}

/* Claim memory for new process p. Do not swap out the currently running
   process in order to get it */
int pagemap_alloc(ptptr p)
{
	return do_pagemap_alloc(p, udata.u_ptab);
}

void pagemap_free(ptptr p)
{
	struct proc_map *m = (struct proc_map *)p->p_page2;
#ifdef CONFIG_IBMPC_EMM
	if (m->emm) {
		emm_free_bank(m);
		return;
	}
#endif
	claim_regions(m, 0);
}

void fork_copy(ptptr p)
{
	struct proc_map *parent_m = (struct proc_map *)udata.u_page2;
	struct proc_map *child_m = (struct proc_map *)p->p_page2;
	/* udata is the parent ptptr p is the child */
#ifdef CONFIG_IBMPC_EMM
	if (emm) {
		/* This one is hard as both might be EMM so we'll need to do
		   a pair of 32K/32K mappings to copy them over */
		emm_copy_data_pages(parent_m, child_m);
	}
	else
#endif
	copy_pages(child->m_dbase, parent->m_dbase, parent->m_dsize);
	/* This last one will need to be reworked once we've got shared
	   code segments */
	copy_pages(child->m_cbase, parent->m_cbase, parent->m_csize);
}


#ifdef CONFIG_SWAP

int swapout(ptptr p)
{
}

void swapin(ptptr p)
{
}

#else /* CONFIG_SWAP */ 
ptptr swapneeded(ptptr p, int f)
{
	return NULL;
}

#endif

#endif
