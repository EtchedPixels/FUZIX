/*
 *	Parameters
 *	PAGE_SIZE -	size of pages (must be at least 1K currently)
 *	PAGE_SHIFT -	bytes to pages
 *
 *	NBANK     -	max number of page slots for the address space
 *	NCACHE	  -	number of cache slots
 *	NPAGE	  -	number of pages (0xFF cannot be used)
 *
 *	TODO
 *	-	Figure out how to work out when to swap in a page and
 *		when it's a new use of that page 
 */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <flat_small.h>

#define DEBUG

extern struct u_data *udata_shadow;

#ifdef CONFIG_FLAT_SMALL

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

/* Not clear we can't get rid of this */
struct meminfo {
	uint8_t low;
	uint8_t high;
	uaddr_t stackbot;
	uaddr_t stacktop;
};

#define NO_PAGE		0xFF
#define NO_SLOT		0xFF
#define SLOT_ANY	0xFF

static uaddr_t page_base;

static uint16_t top_bank;

static struct mem mem[NBANK];
static struct meminfo meminfo[PTABSIZE];
static uint8_t pagemap[PTABSIZE][NBANK];
static uint8_t newmap[NBANK];

static uint8_t allocation[NPAGE / 8];	/* We mark page 0 always busy */
static uint8_t freepages;

/* Recent free pages cache, avoids most bitmap traffic */
static uint8_t cache[NCACHE];
static uint8_t cacheptr = 0;

/* So we walk the bitmap circularly */
static uint8_t *pagen = allocation;

static void dump_map(const char *p)
{
	uint8_t *mp;
	unsigned i;
	kprintf("%s: map [ ", p);
	mp = &pagemap[udata.u_page][0];
	for (i = 0; i < top_bank; i++)
		kprintf("%2x ", *mp++);
	kprintf(" ]\n");
	kprintf("phys map [ ");
	for (i = 0; i < top_bank; i++)
		kprintf("%2x ", mem[i].page);
	kprintf(" ]\n");
}

/* Allocate a page entry. The caller provided the byte
   holding a free page */
static uint8_t pagebits(uint8_t * p)
{
	uint8_t n = *p;
	uint8_t i = 7;
	/* We never pass in 0xFF */
	while (n & 0x80) {
		n <<= 1;
		i--;
	}
	*p |= (1 << i);
//	kprintf("AP: main %x\n", ((p - allocation) << 3) | i);
	return ((p - allocation) << 3)| i;
}

/* Page allocation. We keep a tiny cache and a bitmap. We don't
   deal with shared pages yet */
static uint_fast8_t alloc_page(void)
{
	uint8_t *pb = pagen;
	sysinfo.swapusedk += PAGE_SIZE >> 10;
	freepages--;
	if (cacheptr) {
//		kprintf("AP: cache %x\n", cache[cacheptr - 1]);
		return cache[--cacheptr];
	}
	do {
		if (*pagen != 0xFF)
			return pagebits(pagen);
		pagen++;
		if (pagen == allocation + sizeof(allocation))
			pagen = allocation;
	} while(pagen != pb);
	/* Should never occur as we page count */
	panic("ap: none");
}

/* We only ever free pages that are mapped in. It
   would be easy enough to handle slot 0xFF as a flag
   otherwise but it's not needed */
static void free_page(uint_fast8_t page, uint_fast8_t slot)
{
	/* Handling shared pages would require making this
	   smarter */
	struct mem *mp = mem + slot;
	sysinfo.swapusedk += PAGE_SIZE >> 10;
//	kprintf("Free page %x from slot %d\n", page, slot);
	if (P_PAGE(mp) != page)
		panic("pgfree");
	mp->page = NO_PAGE;
	freepages++;
	if (cacheptr < NCACHE)
		cache[cacheptr++] = page;
	else
		allocation[page >> 3] &= ~(1 << (page & 7));
}

/* These helpers will probably eventually belong in some general collection
   of stuff for paging platforms */

uint16_t swappage;
/* We can re-use udata.u_block and friends as we can never be swapped while
   we are in the middle of an I/O (at least for now). If we rework the kernel
   for sleepable I/O this will change */

int pageread(uint16_t dev, blkno_t blkno, usize_t nbytes,
                    uaddr_t buf, uint16_t page)
{
	int res;

	udata.u_dptr = swap_map(buf);
	udata.u_block = blkno;
	if (nbytes & BLKMASK)
		panic("swprd");
	udata.u_nblock = nbytes >> BLKSHIFT;
	swappage = page;
#ifdef DEBUG
	kprintf("PR | L %p M %p Bytes %u Page %u Block %u\n",
		buf, udata.u_dptr, nbytes, page, udata.u_block);
#endif
	res = ((*dev_tab[major(dev)].dev_read) (minor(dev), 2, 0));

	kprintf("RV %2x %2x\n",
		*(uint8_t *)buf,
		*(uint8_t *)(buf + 1));
		
#ifdef CONFIG_TRIM
	while (nbytes != 0)
	{
		d_ioctl(dev, HDIO_TRIM, (void*)&blkno);
		blkno++;
		nbytes -= 1<<BLKSHIFT;
	}
#endif

	return res;
}

int pagewrite(uint16_t dev, blkno_t blkno, usize_t nbytes,
		     uaddr_t buf, uint16_t page)
{
	/* FIXME: duplication here */
	udata.u_dptr = swap_map(buf);
	udata.u_block = blkno;
	if (nbytes & BLKMASK)
		panic("swpwr");
	udata.u_nblock = nbytes >> BLKSHIFT;
	swappage = page;

	kprintf("WV %2x %2x\n",
		*(uint8_t *)buf,
		*(uint8_t *)(buf + 1));
#ifdef DEBUG
	kprintf("PW | L %p M %p Bytes %u Page %u Block %u\n",
		buf, udata.u_dptr, nbytes, page, udata.u_block);
#endif
	return ((*dev_tab[major(dev)].dev_write) (minor(dev), 2, 0));
}

/* Pages are 1:1 in swap by number. Saves tracking and
   allocating and is fine for our small physical map space */
static void swap_in_page(uint_fast8_t slot, unsigned page)
{
	kprintf("swap in %x at %d\n", page, slot);
	/* Mark us present in the map */
	mem[slot].page = page;
	/* Just arrived */
	mem[slot].age = 0x80;
	/* Assumes a flat map */
	pageread(PAGEDEV, page << (PAGE_SHIFT - BLKSHIFT),
		PAGE_SIZE, PAGE_ADDR(slot), 0);
}

static void swap_out_page(uint_fast8_t slot, unsigned page)
{
	kprintf("swap out %x at %d\n", page, slot);
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

	dump_map("ip");

	for (i = 0; i < top_bank; i++) {
		uint_fast8_t pg = P_PAGE(m);
		/* Found it */
		if (pg == page)
			return i;
		/* Found a free page */
		else if (m->page == NO_PAGE) {
			if (freep != slot || slot == NO_PAGE)
				freep = i;
		} else if (m->age < oldest->age && !(m->page & P_LOCK)) {
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

/* FIXME: work flag so we know if anything is needed */
/* Start with a 1:1 mapping */
static void init_exchanges(void)
{
	uint_fast8_t i = 0;
	uint8_t *mp = newmap;
	while (i < top_bank)
		*mp++ = i++;
}

/* Flip the table entries over. We do all the exchanges
   on the indexes to avoid excess copies/swaps and then
   move them all at the end */
static void queue_exchange(uint_fast8_t p1, uint_fast8_t p2)
{
	uint8_t tmp;
	if (p1 != p2) {
		tmp = newmap[p1];
		newmap[p1] = newmap[p2];
		newmap[p2] = tmp;
	}
}

/*
 *	Do the hard work of making a page present but
 *	not necessarily yet in the right place. Instead
 *	remember the work needed to get that fixed.
 */

static uint_fast8_t make_present(uint_fast8_t page, uint_fast8_t s)
{
	unsigned n = is_present(page, s);
	struct mem *m = mem + n;

	kprintf("make_present %x at %d\n", page, s);
	/* We are present */
	if (P_PAGE(m) == page) {
		/* If we are in present but in the wrong place just
		   queue a swap */
		m->age |= 0x80;
		if (s != SLOT_ANY && n != s) {
			kprintf("present - exchange %d %d\n", n, s);
			queue_exchange(n, s);
		} else
			kprintf("page %d was present at %d\n", page, n);
		return n;
	}
	/* All memory is pinned down */
	if (n == NO_SLOT)
		panic("allpin");
	/* The selected slot contains a page already. This means
	   our page was not present and there were no free slots.
	   Write out the old page and mark it empty */
	if (m->page != NO_PAGE) {
		swap_out_page(n, P_PAGE(m));
		m->page = NO_PAGE;
	}
	/* If a slot was specified then n is the slot that is
	   next to go and is now empty. Swap the current data
	   in the slot we want with the free slot and mark
	   the slot for a page in after the shuffle */
	if (s != SLOT_ANY) {
		if (n != s) {
			kprintf("exchange %d %d\n", n, s);
			queue_exchange(n, s);
		}
		kprintf("then swap into %d\n", s);
		newmap[s] = NO_PAGE;	/* Force a swap in */
	} else {
		/* We need the page somewhere but where doesn't matter
		   Just swap it into the page we got handed and mark
		   it as done */
		/* We can swap it in now, and in fact have to as
		   we won't have the info to swap it in later */
		kprintf("swapin to any %x at %d\n", page, n);
		swap_in_page(n, page);
		m->age |= 0x80;
		newmap[n] = n;	/* Mark map entry done */
	}
	if (s == SLOT_ANY)
		kprintf("page %d made present at %d\n", page, n);
	return n;
}

/*
 *	Perform the queued work to get the maps right
 */
static void perform_swaps(ptptr p)
{
	struct mem *m = mem;
	uint8_t *mp = &pagemap[p->p_page][0];
	unsigned i;

	for (i = 0; i < top_bank; i++) {
		if (newmap[i] != i && newmap[i] != NO_PAGE) {
			struct mem *n = mem + newmap[i];
			uint8_t t;
			t = m->age;
			m->age = n->age;
			n->age = t;
			t = m->page;
			m->page = n->page;
			n->page = t;
			/* TODO: spot one way copies nicely */
			swap_blocks((void *)PAGE_ADDR(i),
				    (void *)PAGE_ADDR(newmap[i]), PAGE_SIZE >> 9);
			/* The same operation as queue to flip the pairs back so we know
			   the work is done */
			kprintf("swapped %d and %d\n",
				i, newmap[i]);
			queue_exchange(i, newmap[i]);
		}
		m++;
	}
	/* Now swap in missing pages */
	for (i = 0; i < top_bank; i++) {
		if (newmap[i] == NO_PAGE && *mp != NO_PAGE) {
			swap_in_page(i, *mp);
			m->age = 0x80;
		}
		mp++;
	}
	dump_map("pswap end");
}

static void age_pages(void)
{
	struct mem *m = mem;
	unsigned i;
	for (i = 0; i < top_bank; i++) {
		m->age >>= 1;
		m++;
	}
}

static void map_pages(ptptr p)
{
	uint8_t *mp = &pagemap[p->p_page][0];
	struct mem *m = mem;
	uint_fast8_t i;

	init_exchanges();
	age_pages();

	for (i = 0; i < top_bank; i++) {
		if (*mp != NO_PAGE)
			make_present(*mp, i);
		mp++;
	}
	perform_swaps(p);

	m = mem;
	mp = &pagemap[p->p_page][0];

	/* Busy marking and lock pages */
	for (i = 0; i < top_bank; i++) {
		if (*mp++ != NO_PAGE) {
			m->page |= P_LOCK;
			m->age >>= 1;
			m->age |= 0x80;
		}
		m++;
	}
}

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

void realloc_map(uint8_t low, uint8_t high)
{
	/* Maybe check pages needed to add versus total swap
	   for oom case */
	/* Then sweep */
	uint8_t *mp = &pagemap[udata.u_page][0];
	uint_fast8_t i;

	kprintf("realloc map %d %d (top %d)\n", low, high, top_bank);
	for (i = 0; i < low; i++) {
		if (*mp == NO_PAGE)
			*mp = alloc_page();
		mp++;
	}
	while (i < high) {
		if (*mp != NO_PAGE) {
			free_page(*mp, i);
			*mp = NO_PAGE;
		}
		mp++;
		i++;
	}

	while (i < top_bank) {
		if (*mp == NO_PAGE)
			*mp = alloc_page();
		mp++;
		i++;
	}

	dump_map("realloc - before map pages");
	/* We still need to make the pages in. We've just
	   allocated any extra no more */
	map_pages(udata.u_ptab);
	dump_map("post mp");
}

void copy_map(uint_fast8_t from, uint_fast8_t to)
{
	uint8_t op;
	uint_fast8_t slot;
	uint_fast8_t n;
	
	kprintf("copy map from %d to %d\n", from, to);

	slot = make_present(from, SLOT_ANY);
	if (slot == NO_SLOT)
		panic("cmap");
	op = mem[slot].page;
	mem[slot].page |= P_LOCK;
	/* Make the child page appear somewhere, anywhere */
	n = make_present(to, SLOT_ANY);
	kprintf("copy blocks to %d:%p from %d:%p for %d\n",
		n, PAGE_ADDR(n), slot, PAGE_ADDR(slot), PAGE_SIZE >> 9);
		
	copy_blocks((void *)PAGE_ADDR(n), (void *)PAGE_ADDR(slot), PAGE_SIZE >> 9);
	/* Unlock if was not locked */
	mem[slot].page = op;
	/* So we favour in position parent pages for parent first */
	mem[n].age >>= 1;
}

static uint_fast8_t map_copy(ptptr p, ptptr c)
{
	uint_fast8_t i;
	uint8_t *pp = &pagemap[p->p_page][0];
	uint8_t *cp = &pagemap[c->p_page][0];
	struct meminfo *mi = meminfo + p->p_page;

	if (top_bank - mi->high + mi->low > freepages)
		return ENOMEM;

	/* To allow all memory to be mapped we need to unlock
	   all pages here and then cycle pinning pages and unpinning
	   as we fork */
	unlock_pages();
	kprintf("forking %p to %p pmap %d to %d\n",
		pp, cp, p->p_page, c->p_page);
	for (i = 0; i < top_bank; i++) {
		if (*pp != NO_PAGE) {
			*cp = alloc_page();
			/* This will have to handle paging and pinning both pages */
			copy_map(*pp, *cp);
		} else
			*cp = NO_PAGE;
		pp++;
		cp++;
	}
	/* We run parent first */
	/* Put back the map and lock it */
	dump_map("post copy");
	map_pages(p);
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

	if (p->p_pid == 1) {
#ifdef udata
		udata_shadow = p->p_udata;
#endif
		kprintf("Making init pb %p\n", page_base);
		/* Manufacturing init */
		pt = &pagemap[p->p_page][0];
		mi->low = 0;
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
   to us, but matters to other mappers */
void pagemap_switch(ptptr p, int death)
{
	map_pages(p);
}

/* Called on exit */
void pagemap_free(ptptr p)
{
	/* Actually always called with p as the current process */
	uint8_t *mp = &pagemap[udata.u_page][0];
	uint_fast8_t i;

	for (i = 0; i < top_bank; i++) {
		if (*mp != NO_PAGE)
			free_page(*mp, i);
		mp++;
	}
}

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

	if (nl + nh - has > freepages) {
		kprintf("needed %d have %d\n", nl + nh - has, freepages);
		return ENOMEM;
	}

	/* This also maps the pages as desired */
	realloc_map(nl, top_bank - nh);
	m->high = top_bank - nh;
	m->low = nl;
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

	kprintf("realloc ok: cb %p stkbot %p, stktop %p\n",
		udata.u_codebase, m->stackbot, m->stacktop);

	return 0;
}

usize_t pagemap_mem_used(void)
{
	uint_fast8_t i, ct = 0;
	struct mem *m = mem;
	for (i = 0; i < top_bank; i++) {
		if (m->page != NO_PAGE)
			ct++;
	}
	/* TODO: Assumes block size is >= 1K */
	return ct << (PAGE_SHIFT - 10);
}

usize_t valaddr(const uint8_t * pp, usize_t l)
{
	struct meminfo *m = meminfo + udata.u_page;
	usize_t n = 0;
	uaddr_t p = (uaddr_t) pp;

	/* Code/Data/BSS/Break */
	if (p >= udata.u_codebase && p < udata.u_break)
		n = udata.u_break - p;
	/* Stack (high) */
	else if (p >= m->stackbot && p <= m->stacktop)
		n = m->stacktop - p;
	if (n > l)
		n = l;
	if (n)
		return n;
	kprintf("addr %p not valid base %p end %p\n",
		p, udata.u_codebase, udata.u_break);
	udata.u_error = EFAULT;
	return 0;
}

/* Size must be a multiple of 8 pages for simplicity */
/* FIXME: Needs a better name as we give it disk blocks for size */
void pagemap_frames(unsigned size)
{
	/* Disk blocks to bytes */
	size <<= BLKSHIFT;
	sysinfo.swapk += size >> 10;
	size >>= PAGE_SHIFT + 3;
	if (size < NPAGE)
		memset(allocation + size, 0xFF, NPAGE - size);
	/* The first page was pre-emptively borrowed for init so mark it
	   as busy */
	allocation[0] = 0x01;
	kprintf("Swap: %d pages.\n", size * 8);
	freepages = size;
}

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
}

arg_t brk_extend(uaddr_t addr)
{
	struct meminfo *mi = meminfo + udata.u_page;
	uint8_t *m = &pagemap[udata.u_page][0];
	uint_fast8_t nl;
	int i;

	if (addr < udata.u_codebase)
		return EINVAL;
	if (addr >= mi->stackbot - 512)
		return ENOMEM;

	kprintf("brk_extend %p\n", addr);
	dump_map("extend");
	/* Fill in the extra pages */
	nl = (addr - page_base + PAGE_SIZE - 1) >> PAGE_SHIFT;
	if (nl - mi->low > freepages)
		return ENOMEM;
	/* Fill in the pages */
	for (i = mi->low; i < nl; i++) {
		if (m[i] == NO_PAGE)
			m[i] = alloc_page();
	}
	mi->low = nl;
	/* TODO: do we need to unpin first ? */
	/* and map them */
	map_pages(udata.u_ptab);
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
	map_pages(udata.u_ptab);
	return 0;
}

/* No memalloc */
arg_t _memalloc(void)
{
	udata.u_error = ENOMEM;
	return -1;
}

arg_t _memfree(void)
{
	udata.u_error = ENOMEM;
	return -1;
}


#endif
