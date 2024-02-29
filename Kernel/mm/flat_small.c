/*
 *	A software MMU and paging implementation for smaller address
 *	spaces. This is designed for machines where the cost of copying
 *	memory around is signifcantly below the cost of disk I/O - such
 *	as when running bitbang SD cards.
 *
 *	All applications see a classic Unix address space where the stack
 *	is at the top of the user memory space and the program grows upwards.
 *	There is provision for stack growth but this isn't tested as it would
 *	need some compiler hackery to do stack grow syscalls.
 *
 *	Assumptions
 *	-	Single address space. Pages exist either on disk or in
 *		the mapped address space. There is no provision for them
 *		to be elsewhere
 *	-	Platform has a pure a_text. If that isn't the case then
 *		you'll need to add defines to disable sharing.
 *
 *	TODO:
 *	-	Review the pathalogical edge cases when we are using
 *		all of the map pages and we have brk and stack into the
 *		same page.
 *
 *	Parameters
 *	PAGE_SIZE -	size of pages (must be at least 1K currently)
 *	PAGE_SHIFT -	bytes to pages
 *
 *	NBANK     -	max number of page slots for the address space
 *	NPAGE	  -	number of pages (max 0x7E)
 *
 */

#include <kernel.h>

#ifdef CONFIG_FLAT_SMALL

#include <kdata.h>
#include <printf.h>
#include <page.h>
#include <flat_small.h>


#undef DEBUG

extern struct u_data *udata_shadow;


#ifndef PAGE_SIZE
#define PAGE_SIZE	1024
#define PAGE_SHIFT	10
#endif

#define PAGE_ADDR(x)	(((x) << PAGE_SHIFT) + page_base)

/*
 *	We keep one of these for each mapped block
 */

struct mem {
	uint8_t page;
#define P_LOCK	0x80
#define P_PAGE(x)	(((x)->page) & 0x7F)
	uint8_t age;
};

struct meminfo {
	uint8_t texttop;	/* Shared to this point */
	uint8_t low;
	uint8_t high;
	uaddr_t stackbot;
	uaddr_t stacktop;
};

#define NO_PAGE		0xFF
#define NO_SLOT		0xFF
#define SLOT_ANY	0xFF
#define INVALID_PAGE	0x7F	/* Not valid as with P_LOCK becomes NO_PAGE */

/* The address of the start of the user memory region. Set by the platform
   during boot. Alignment is the platform's problem. We dont care */

static uaddr_t page_base;

/* The highest usable bank number. Set based on the memory free. If NBANK
   it too low it will be capped at NBANK */
static uint16_t top_bank;

/* Mem tracks the current page in each address range. its age and also
   whether it is pinned or not */
static struct mem mem[NBANK];
/* Memory manager specific data for each process */
static struct meminfo meminfo[PTABSIZE];
/* Memory map for each process - simple linear arrays as the address space
   is small */
static uint8_t pagemap[PTABSIZE][NBANK];

/* Reverse mapping table. In theory we could combine this with the allocation
   map at some point and maybe save space at a small scanning cost. Also our
   free page table. This works with shared pages because the page will only
   ever be in one map location */
static uint8_t rmap[NPAGE];
static uint_fast8_t freepages;
static uint8_t *freeptr = rmap;

#define SWAPPED	0xF0		/* On disk */
#define EMPTY 0xF8		/* No content */
#define FREE 0xFF		/* Can be allocated */

#ifdef DEBUG

static unsigned shared_page(uint8_t *pp, uint_fast8_t slot);

/* Dump the map state */
static void dump_map(const char *p)
{
	uint8_t *mp;
	unsigned i;
	kprintf("%s: map [ ", p);
	mp = &pagemap[udata.u_page][0];
	for (i = 0; i < top_bank; i++) {
		if (shared_page(mp, i))
			kputchar('S');
		else
			kputchar('P');
		kprintf("%2x ", *mp++);
	}
	kprintf(" ]\n");
	kprintf("phys map [ ");
	for (i = 0; i < top_bank; i++) {
		kprintf("%2x ", mem[i].page);
		if (mem[i].page != NO_PAGE && rmap[P_PAGE(mem + i)] != i)
			kprintf("\nRMAP bad %2x\n",
				rmap[P_PAGE(mem + i)]);
	}
	kprintf(" ]\n");
}
#else
#define dump_map(x)	do {} while(0);
#endif

/* Page allocation. We don't deal with shared pages yet */
static uint_fast8_t alloc_page(void)
{
	sysinfo.swapusedk += PAGE_SIZE >> 10;
	if (freepages == 0)
		panic("ap: no pages");
	freepages--;
	/* We checked the free pages count first so this
	   will terminate */
	while(*freeptr != FREE) {
		freeptr++;
		if (freeptr == rmap + NPAGE)
			freeptr = rmap;
	}
	*freeptr = EMPTY;
	return freeptr - rmap;
}

/*
 *	We share pages by fork() and we have no messy BSD style
 *	mmap() to worry about. This means that the page can only
 *	exist in the same slot in the other maps.
 */
static unsigned shared_page(uint8_t *pp, uint_fast8_t slot)
{
	uint8_t *p = &pagemap[0][slot];
	uint8_t *e = p + sizeof(pagemap);
	uint8_t page = *pp;

	while(p < e) {
		if (*p == page && p != pp)
			return 1;
		p += NBANK;
	}
	return 0;
}

/* Reuse a page unless it is shared, in which case we allocate ourselves
   a new one */
static void unshare_page(uint_fast8_t *pp, uint_fast8_t slot)
{
	if (shared_page(pp, slot))
		*pp = alloc_page();
}

/* We only ever free pages that are mapped in. It
   would be easy enough to handle slot 0xFF as a flag
   otherwise but it's not needed */
static void free_page(uint_fast8_t *pp, uint_fast8_t slot, unsigned unshared)
{
	/* Handling shared pages would require making this
	   smarter */

	struct mem *mp = mem + slot;
	if (!unshared && shared_page(pp, slot)) {
		/* Remove from this map but leave elsewhere */
		*pp = NO_PAGE;
		return;
	}

	sysinfo.swapusedk -= PAGE_SIZE >> 10;
	if (P_PAGE(mp) != *pp)
		panic("pgfree");
	if (rmap[*pp] != slot)
		panic("pgfree2");
	mp->page = NO_PAGE;
	rmap[*pp] = FREE;
	*pp = NO_PAGE;
	freepages++;
}

/* Pages are 1:1 in swap by number. Saves tracking and
   allocating and is fine for our small physical map space */
static void swap_in_page(uint_fast8_t slot, unsigned page)
{
	/* Mark us present in the map */
	mem[slot].page = page;
	rmap[page] = slot;
	/* Just arrived */
	mem[slot].age = 0x80;
	/* Assumes a flat map */
	pageread(PAGEDEV, page << (PAGE_SHIFT - BLKSHIFT),
		PAGE_SIZE, PAGE_ADDR(slot), 0);
}

static void swap_out_page(uint_fast8_t slot, unsigned page)
{
	/* Assumes a flat map */
	pagewrite(PAGEDEV, page << (PAGE_SHIFT - BLKSHIFT),
		PAGE_SIZE, PAGE_ADDR(slot), 0);
}

/*
 *	Does the monkey work for mapping a page. We figure out
 *	simultaneously if the page is present, where it is present
 *	if there is a free space, if there is an ideal free space
 *	and if not who to swap to disk. We don't however actually
 *	make any changes to anything
 */
static uint_fast8_t is_present(uint_fast8_t page, uint_fast8_t slot)
{
	struct mem *m = mem;
	struct mem *oldest = mem;
	uint_fast8_t oldn = NO_SLOT;
	uint_fast8_t freep = NO_SLOT;
	uint_fast8_t i;

	if (rmap[page] < SWAPPED)
		return rmap[page];

	for (i = 0; i < top_bank; i++) {
		if (m->page == NO_PAGE) {
			if (freep != slot || slot == NO_PAGE)
				freep = i;
		/* needs to be <= as the first time we'll compared node
		   0 with itself */
		} else if (m->age <= oldest->age && !(m->page & P_LOCK)) {
			oldest = m;
			oldn = i;
		}
		m++;
	}
	/* First preference if not present - a free page */
	if (freep != NO_PAGE)
		return freep;
	/* Swap victim */
	return oldn;
}

/*
 *	Do in software what a real MMU does in hardware. Swap
 *	pages and move pages around memory/
 */
static void exchange_pages(uint_fast8_t p1, uint_fast8_t p2)
{
	struct mem *m1 = mem + p1;
	struct mem *m2 = mem + p2;
	struct mem tmp;

#ifdef DEBUG
	kprintf("swap %d (%x) with %d (%x)\n",
		p1, m1->page, p2, m2->page);
#endif

	swap_blocks((void *)PAGE_ADDR(p1),
		    (void *)PAGE_ADDR(p2), PAGE_SIZE >> 9);
	tmp.page = m1->page;
	tmp.age = m1->age;
	m1->page = m2->page;
	m1->age = m2->age;
	m2->page = tmp.page;
	m2->age = tmp.age;
	rmap[P_PAGE(m1)] = p1;
	rmap[P_PAGE(m2)] = p2;
}

static void move_page(uint_fast8_t to, uint_fast8_t from)
{
	struct mem *m1 = mem + to;
	struct mem *m2 = mem + from;

#ifdef DEBUG
	kprintf("move %d (%x) to %d (%x)\n",
		from, m2->page, to, m1->page);
#endif

	copy_blocks((void *)PAGE_ADDR(to), (void *)PAGE_ADDR(from),
		PAGE_SIZE >> 9);
	if (m1->page != NO_PAGE)
		rmap[P_PAGE(m1)] = SWAPPED;
	m1->age = m2->age;
	m1->page = m2->page;
	rmap[P_PAGE(m1)] = to;
	m2->page = NO_PAGE;
}

/*
 *	Do the hard work of making a page present. We may be required
 *	to swap it in or maybe not.
 */
static uint_fast8_t make_present(uint_fast8_t page, uint_fast8_t s, unsigned swap)
{
	unsigned n = is_present(page, s);
	struct mem *m = mem + n;

#ifdef DEBUG
	kprintf("make_present %x at %d (%d): ", page, s, swap);
#endif

	/* We are present */
	if (P_PAGE(m) == page) {
		m->age |= 0x80;
		/* Don't care or right place */
		if (s == SLOT_ANY || s == n) {
#ifdef DEBUG
			kprintf("page was present at %d\n", n);
#endif
			return n;
		}
		/* Now in the right place */
		/* If we are switching an empty page then just copy */
		if (mem[s].page == NO_PAGE)
			move_page(s, n);
		else
			exchange_pages(n, s);
#ifdef DEBUG
		kprintf("page %d exchanged into %d\n", page, s);
#endif
		return s;
	}
	/* All memory is pinned down */
	if (n == NO_SLOT)
		panic("allpin");

	/* We are not present. See if we've been handed a free page and
	   can use it */
	if (m->page != NO_PAGE) {
		/* We've been handed the oldest unlocked page */
		/* Free the slot by paging out the old user */
		swap_out_page(n, P_PAGE(m));
		rmap[P_PAGE(m)] = SWAPPED;
	}
	if (s != SLOT_ANY && s != n) {
		/* We have a free page but the wrong one. Swap whatever
		   was there for our page. We might end up causing several
		   excess page swaps but that can be optimized later if
		   we need to address it */
		move_page(n, s);
		n = s;
		m = mem + n;
	}
	if (swap) 
		swap_in_page(n, page);
	m->page = page;
	m->age = 0x80;
	rmap[page] = n;
#ifdef DEBUG
	kprintf("page %d made present at %d\n", page, n);
#endif
	return n;
}

/*
 *	Least regularly used page aging (the other better LRU)
 *	Pages run a decaying age so that a page used more often
 *	in the past wins over a page of the same age that was not.
 */
static void age_pages(void)
{
	struct mem *m = mem;
	unsigned i;
	for (i = 0; i < top_bank; i++) {
		m->age >>= 1;
		m++;
	}
}

/*
 *	Make the physical map algin with our page map
 */
static void map_pages(ptptr p, unsigned pagein)
{
	uint8_t *mp = &pagemap[p->p_page][0];
	struct mem *m = mem;
	uint_fast8_t i;

	age_pages();

	/* Mark all of our pages as most recent so that they don't
	   get thrashed */
	for (i = 0; i < top_bank; i++) {
		if (*mp != NO_PAGE && rmap[*mp] < SWAPPED) {
			mem[rmap[*mp]].age |= 0x80;
		}
		mp++;
	}

	mp = &pagemap[p->p_page][0];

	for (i = 0; i < top_bank; i++) {
		if (*mp != NO_PAGE)
			make_present(*mp, i, pagein);
		mp++;
	}
	m = mem;
	mp = &pagemap[p->p_page][0];

	/* Busy marking and lock pages */
	for (i = 0; i < top_bank; i++) {
		if (*mp++ != NO_PAGE) {
			m->page |= P_LOCK;
			m->age |= 0x80;
		}
		m++;
	}
}

/*
 *	Clear all the lock flags.
 */
static void unlock_pages(void)
{
	struct mem *m = mem;
	uint_fast8_t i;

	for (i = 0; i < top_bank; i++) {
		if (m->page != NO_PAGE)
			m->page &= ~P_LOCK;
		m++;
	}
}

/*
 *	The user has done an execve. We have a bunch of pages
 *	we own that held the old binary and are convenietly
 *	in memory in the right places to reuse. Complete the job
 *	by trimming excess pages or adding pages to fill the gaps.
 *
 *	Our caller has all our pages in use pinned and present so
 *	we can safely remap without asking for swapin
 */
void realloc_map(uint8_t low, uint8_t high, uint8_t oldshared)
{
	/* Maybe check pages needed to add versus total swap
	   for oom case */
	/* Then sweep */
	uint8_t *mp = &pagemap[udata.u_page][0];
	uint_fast8_t i;

#ifdef DEBUG
	kprintf("realloc map %d %d (top %d)\n", low, high, top_bank);
#endif
	
	for (i = 0; i < low; i++) {
		if (*mp == NO_PAGE)
			*mp = alloc_page();
		else if (i < oldshared)
			unshare_page(mp, i);
		mp++;
	}
	while (i < high) {
		if (*mp != NO_PAGE)
			free_page(mp, i, i >= oldshared);
		mp++;
		i++;
	}
	while (i < top_bank) {
		if (*mp == NO_PAGE)
			*mp = alloc_page();
		else if (i < oldshared)
			unshare_page(mp, i);
		mp++;
		i++;
	}

	dump_map("realloc - before map pages");
	/* We still need to move the pages in. We've just
	   allocated any extra no more. No page in needed */
	map_pages(udata.u_ptab, 0);
	dump_map("post mp");
}

/*
 *	Copy a map. This is trickier than it looks. If we have most
 *	of memory bunged up with our running process we may not actually
 *	have any space to map the page we are copying into. As a result
 *	we have to be prepared to lock and unlock map entries and swap
 *	stuff to make room
 *
 *	TODO: arguably it would be better to spot the case of having
 *	no usable pages and simply swap the existing page out to the swap
 *	entry of the new page.
 */
void copy_map(uint_fast8_t from, uint_fast8_t to)
{
	uint8_t op;
	uint_fast8_t slot;
	uint_fast8_t n;

#ifdef DEBUG
	kprintf("copy map from %d to %d\n", from, to);
#endif

	slot = make_present(from, SLOT_ANY, 1);
	if (slot == NO_SLOT)
		panic("cmap");
	op = mem[slot].page;
	mem[slot].page |= P_LOCK;
	/* Make the child page appear somewhere, anywhere */
	n = make_present(to, SLOT_ANY, 0);
	copy_blocks((void *)PAGE_ADDR(n), (void *)PAGE_ADDR(slot), PAGE_SIZE >> 9);
	/* Unlock if was not locked */
	if (mem[slot].page != (op | P_LOCK))
		panic("cmap");
	mem[slot].page = op;
	/* So we favour in position parent pages for parent first */
	mem[n].age >>= 1;
}

/*
 *	Make a copy of the existing process map to a child.
 */
static uint_fast8_t map_copy(ptptr p, ptptr c)
{
	uint_fast8_t i;
	uint8_t *pp = &pagemap[p->p_page][0];
	uint8_t *cp = &pagemap[c->p_page][0];
	struct meminfo *mi = meminfo + p->p_page;

	if (top_bank - mi->high + mi->low - mi->texttop > freepages)
		return ENOMEM;

	/* To allow all memory to be mapped we need to unlock
	   all pages here and then cycle pinning pages and unpinning
	   as we fork */
	unlock_pages();
	dump_map("pre copy");

	for (i = 0; i < top_bank; i++) {
		if (*pp != NO_PAGE) {
			if (i < mi->texttop)
				*cp = *pp;
			else {
				*cp = alloc_page();
				/* This will have to handle paging and pinning both pages */
				copy_map(*pp, *cp);
			}
		} else
			*cp = NO_PAGE;
		pp++;
		cp++;
	}
	/* We run parent first */
	/* Put back the map and lock it */
	dump_map("post copy");
	map_pages(p, 1);
	dump_map("post fork");
	return 0;
}

/* Called on process creation */

int pagemap_alloc(ptptr p)
{
	struct meminfo *mi, *pmi;
	uint8_t *pt;

	p->p_page = p - ptab;

	if (plt_udata_set(p))
		return ENOMEM;

	mi = meminfo + p->p_page;

	/*
	 *	Create init. This happens early and is a bit special
	 */
	if (p->p_pid == 1) {
#ifdef udata
		udata_shadow = p->p_udata;
#endif
		/* Manufacturing init */
		pt = &pagemap[p->p_page][0];
		mi->low = 0;
		mi->texttop = 0;
		mi->high = top_bank;
		memset(pt, NO_PAGE, NBANK);
		/* This is hairy as we don't have any swap backing set
		   up so we fake it on the basis we'll have at least one
		   page */
		pt[0] = 0;
		udata.u_codebase = page_base;
		udata.u_break = page_base + PAGE_SIZE;
		return 0;
	}
	/* Forking a copy */
	pmi = meminfo + udata.u_page;
	memcpy(mi, pmi, sizeof(*mi));
	return map_copy(udata.u_ptab, p);
}

/* Switch the map to p. Death case is unimportant
   to us, but matters to other mappers. In our case
   if the previous occupant died then we've run through
   pagemap_free and our map is full of empty pages which
   we will manage appropriately */
void pagemap_switch(ptptr p, int death)
{
	dump_map("pre switch");
	unlock_pages();
	map_pages(p, 1);
	dump_map("post switch");
}

/* Called on exit */
void pagemap_free(ptptr p)
{
	/* Actually always called with p as the current process */
	uint8_t *mp = &pagemap[udata.u_page][0];
	uint8_t ttop = meminfo[p->p_page].texttop;
	uint_fast8_t i;

	for (i = 0; i < top_bank; i++) {
		if (*mp != NO_PAGE)
			free_page(mp, i, i >= ttop);
		mp++;
	}
}

/*
 *	Adjust the memory map for a new process. We do the Fuzix housekeeping
 *	here but the actual page managment is dealt with in realloc_map
 */
int pagemap_realloc(struct exec *hdr, usize_t size)
{
	struct meminfo *m = meminfo + udata.u_page;
	int has = m->low + top_bank - m->high;
	int nl, nh;

	/* Get sizes in bytes */
	nl = hdr->a_text + hdr->a_data + hdr->a_bss;
	nh = hdr->stacksize;

	/* Turn them into inclusive pages to cover all the memory */
	nl += PAGE_SIZE - 1;
	nl >>= PAGE_SHIFT;
	nh += PAGE_SIZE - 1;
	nh >>= PAGE_SHIFT;

	if (nl + nh + m->texttop - has > freepages) {
		/* This is slightly pessimistic. In the case this fails
		   we ought to count our shared pages the hard way to make
		   sure TODO */
		return ENOMEM;
	}

	/* This also maps the pages as desired */
	realloc_map(nl, top_bank - nh, m->texttop);
	m->high = top_bank - nh;
	m->low = nl;
	/* A shared page must be totally text so may not have any. Partial
	   pages are not shared */
	m->texttop = hdr->a_text / PAGE_SIZE;

	udata.u_codebase = page_base;
	/* Single mapping for now */
	udata.u_database = udata.u_codebase + hdr->a_text;
	udata.u_break = udata.u_database + hdr->a_data + hdr->a_bss;

	/* Stack map range. Cache for valaddr */
	m->stacktop = PAGE_ADDR(top_bank) - 1;
	m->stackbot = PAGE_ADDR(top_bank - nh);
	
	/* Tell the execve() code to build the stack in the top of
	   our memory space we allocated */
	udata.u_top = PAGE_ADDR(top_bank);

	return 0;
}

/*
 *	Caculate the number of pages of memory used. 
 */
usize_t pagemap_mem_used(void)
{
	uint_fast8_t i, ct = 0;
	struct mem *m = mem;
	for (i = 0; i < top_bank; i++) {
		if (m->page != NO_PAGE)
			ct++;
		m++;
	}
	return ct << (PAGE_SHIFT - 10);
}

#ifndef CONFIG_LEVEL_0
/*
 *	An address is valid if it lies between the start of the code
 *	and the brk address or between the bottom and top of the stack.
 */
usize_t valaddr(const uint8_t * pp, usize_t l, uint_fast8_t is_write)
{
	struct meminfo *m = meminfo + udata.u_page;
	usize_t n = 0;
	uaddr_t p = (uaddr_t) pp;

	/* Code/Data/BSS/Break */
	if (!is_write || p >= udata.u_database) {
		if (p >= udata.u_codebase && p < udata.u_break)
			n = udata.u_break - p;
		/* Stack (high) */
		else if (p >= m->stackbot && p <= m->stacktop)
			n = m->stacktop - p;
		if (n > l)
			n = l;
		if (n)
			return n;
	}
	udata.u_error = EFAULT;
	return 0;
}

usize_t valaddr_r(const uint8_t *pp, usize_t l)
{
	return valaddr(pp, l, 0);
}

usize_t valaddr_w(const uint8_t *pp, usize_t l)
{
	return valaddr(pp, l, 1);
}

#endif

/*
 *	Called by the disk management layers when they find
 *	our page file.
 */
void pagefile_add_blocks(unsigned blocks)
{
	unsigned size = blocks >> (PAGE_SHIFT - BLKSHIFT);

	if (size > NPAGE)
		size = NPAGE;

	sysinfo.swapk = size << (PAGE_SHIFT - 10);

	/* Fill the allocation stack */
	freepages = size - 1 ;
	memset(rmap + 1, FREE, freepages);
}

/*
 *	Called by the platform to set up the address range that is left
 *	for user space. The platform gets to align the base as it wants.
 *	As the page sizes are powers of two the rest will then stay
 *	aligned as desired.
 */
void pagemap_setup(uaddr_t base, unsigned len)
{
	unsigned i;
	len /= PAGE_SIZE;
	if (len > NBANK) {
		len = NBANK;
		kprintf("Not enough page frames for all of memory.\n");
	}
	page_base = base;
	top_bank = len;
	kprintf("%d %dK page frames available @%p.\n",
		top_bank, PAGE_SIZE >> 10, base);
	for (i = 0; i < top_bank; i++)
		mem[i].page = NO_PAGE;
	/* Magic for init setup */
	rmap[0] = 0;
	/* Mark rest of the map used */
	memset(rmap + 1, SWAPPED, NPAGE - 1);
	/* The one page already in use */
	sysinfo.swapusedk = PAGE_SIZE >> 10;
}

/*
 *	The user is trying to change the brk() addresss. Unlike
 *	a lot of mapping models we can actually handle this nicely.
 */
arg_t brk_extend(uaddr_t addr)
{
	struct meminfo *mi = meminfo + udata.u_page;
	uint8_t *m = &pagemap[udata.u_page][0];
	uint_fast8_t nl;
	int i;

	/* Cannot'brk into code */
	if (addr < udata.u_database)
		return EINVAL;
	if (addr >= mi->stackbot - 512)
		return ENOMEM;

	/* Fill in the extra pages */
	nl = (addr - page_base + PAGE_SIZE - 1) >> PAGE_SHIFT;

	if (nl == mi->low)
		return 0;

	if (nl - mi->low > freepages)
		return ENOMEM;

	dump_map("extend");
	/* Fill in the pages */
	for (i = mi->low; i < nl; i++) {
		if (m[i] == NO_PAGE)
			m[i] = alloc_page();
	}
	mi->low = nl;
	/* TODO: do we need to unpin first ? */
	/* and map them */
	map_pages(udata.u_ptab, 1);
	return 0;
}

/* We can do stack grows for platforms with suitable support */

arg_t stack_extend(uaddr_t sp)
{
	struct meminfo *mi = meminfo + udata.u_page;
	uint8_t *m = &pagemap[udata.u_page][0];
	uint_fast8_t nh;
	unsigned i;

	if (sp >= mi->stackbot)
		return 0;

	nh = (mi->stacktop - sp + PAGE_SIZE - 1) >> PAGE_SHIFT;
	/* Stack fault - no fix */
	if (sp <= udata.u_break || nh - mi->high > freepages)
		doexit(SIGKILL << 8);
		/* Won't return */
	for (i = nh; i < mi->high; i++) {
		if (m[i] == NO_PAGE)
			m[i] = alloc_page();
	}
	mi->high = nh;
	map_pages(udata.u_ptab, 1);
	return 0;
}
#endif
