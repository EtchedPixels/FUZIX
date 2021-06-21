/*
 *	Manage a system with 8K pages, and 16bit pointers.
 *
 *	It also supports a 'magic page' to allow graphics page mapping
 *	into a process and the correct behaviour on execve etc.
 *
 *	To use this banker you must be able to set
 *	PAGE_INVALID	a value meaning this page does not map anywhere
 *	PAGE_VIDEO	a page for video mapping (or set to PAGE_INVALID)
 *
 *	Invalid can either be a real 'invalid' for the MMU or can be set
 *	to something like a random chunk of ROM. Anything that stops
 *	scribbles.
 *
 *	The banker will keep an array of 8 or 10 items per process (8 x 8K
 *	banks lus a reference for the video bank underlay)
 *
 *	Set:
 *	CONFIG_BANK8
 *	CONFIG_VIDMAP8	if you have an 8K video mapping option
 *	MAX_MAPS	to the largest number of free pages there can be
 *	SWAPDEV		if using swap
 *	PROGTOP		first byte above process space
 *	SWAPBASE	address we swap from
 *	TOP_SIZE	how much of the top 8K page to swap
 *			(this isn't computed as you may want to add 512 bytes
 *			for the udata)
 *	SWAP_SIZE	total space (in blocks) per swapped process worst case
 *
 *	Page numbers must not include 0 (0 is taken as swapped)
 *
 *	FIXME: currently requires that PROGTOP aligns to an 8K boundary
 *	if using swap.
 *
 *	Also we kind of assume that program space starts at 0x0000. That
 *	needs fixing.
 */
#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <bank8k.h>
#include <exec.h>

#ifdef CONFIG_BANK8

#ifdef CONFIG_VIDMAP8
#define PTNUM		9
#define PTSIZE		10
#else
#define PTSIZE		8
#define PTNUM		8
#define PAGE_VIDEO	PAGE_INVALID
#endif

/* Bank numbers we actually use */
#define HIBANK ((uint8_t)((((PROGTOP) + 0x1FFFUL) >> 13)))
#define LOBANK ((PROGBASE) >> 13)

static uint8_t pfree[MAX_MAPS];
static uint8_t pfptr = 0;
static uint8_t pmap[PTABSIZE][PTSIZE];

#ifdef CONFIG_VIDMAP8

int vidmap_unmap(void)
{
	uint8_t *p = (uint8_t *)udata.u_page;
	if (p[8] == PAGE_INVALID) {
		udata.u_error = EINVAL;
		return -1;
	}
	p[p[9]] = p[8];
	p[8] = PAGE_INVALID;
	return 0;
}

/* Caller must validate addr */
int vidmap_map(uint16_t addr)
{
	uint8_t *p = (uint8_t *)udata.u_page;
	uint8_t s = addr >> 13;
	if (p[8] != PAGE_INVALID) {
		udata.u_error = EINVAL;
		return -1;
	}
	p[8] = p[s];
	p[9] = s;
	p[s] = PAGE_VIDEO;
	return 0;
}

#endif

/*
 * Map handling
 */

/*
 *	Helper for platform to add pages at boot
 */
void pagemap_add(uint8_t page)
{
	if (pfptr == MAX_MAPS)
		panic(PANIC_MAPOVER);
	pfree[pfptr++] = page;
}

/*
 *	Free the maps for this task
 *
 *	FIXME: we need to change this to consider the case where
 *	we have entirely spare 8K pages mapped above the common. Right now
 *	this code breaks if PROGTOP is < E000.
 */
void pagemap_free(ptptr p)
{
	uint8_t *pt = (uint8_t *)p->p_page;
	uint8_t *e = (uint8_t *)p->p_page + PTNUM - LOBANK;
	uint8_t last = PAGE_INVALID;

	while(pt < e) {
		if (*pt != PAGE_INVALID && *pt != PAGE_VIDEO && *pt != last) {
			last = *pt;
			pfree[pfptr++] = *pt;
		}
		pt++;
	}
}

/*
 *	We are passed the number of bytes that are needed for this
 *	process. On top of that we require the extra space between
 *	the top of space and PROGTOP which is reserved for the common
 *	space and udata. If you have no need for this then it'll come out
 *	as size - 1, which is correct.
 *
 *	We calculate how many banks you need from address 0 and then
 *	adjust for LOBANK
 */
static int maps_needed(uint16_t top)
{
	/* On many platforms if you touch this or PROGTOP you must
	   touch tricks.s */
	uint16_t needed = top + 0xFFFF - PROGTOP;
	/* Usually we have 0x1000 common - 1 for shift and inc */
	needed >>= 13;		/* in banks */
	needed++;		/* rounded */
	needed -= LOBANK;	/* if we have free space below we don't need
				   those maps */
	return needed;
}

/*
 *	Allocate the maps for this task post fork
 *	We have a hackish fix for init that would be nice
 *	resolve.
 *
 *	Use p-> not udata. The udata is not yet swapped in!
 */
int pagemap_alloc(ptptr p)
{
	uint8_t *ptr;
	int needed = maps_needed(p->p_top);
	int i;

	/* Cheapest way to keep it non zero */
	ptr = pmap[p - ptab];
	p->p_page = (uint16_t)ptr;

#ifdef SWAPDEV
	/* Throw our toys out of our pram until we have enough room */
	while (needed > pfptr)
		if (swapneeded(p, 1) == NULL)
			return ENOMEM;
#else
	if (needed > pfptr)	/* We have no swap so poof... */
		return ENOMEM;
#endif

	/* Allocate the pages upwards */
	for (i = 0; i < needed; i++)
		*ptr++ = pfree[--pfptr];
	/*
	 *	A machine with a separate supervisor space should write
	 *	the unused pages to invalid so they fault. A system without
	 *	however we need to replicate the top page number so the
	 *	common space is right if it is high.
	 */
#ifdef CONFIG_SUPERVISOR_SPACE
	while (i++ < PTNUM - LOBANK)
		*ptr++ = PAGE_INVALID;
#else
	while (i++ < 8 - LOBANK) {
		*ptr = ptr[-1];
		ptr++;
	}
#ifdef CONFIG_VIDMAP8
	ptr += LOBANK;
	*ptr = PAGE_INVALID;
#endif	
#endif
	return 0;
}

/*
 *	Reallocate the maps for a process
 *
 *	Currently only called on execve and we rely on this in the
 *	specific case of bank8k.
 *
 *	FIXME: needs updating once we have all the new sizes/stack etc right
 */
int pagemap_realloc(struct exec *hdr, usize_t size)
{
	int8_t have = maps_needed(udata.u_ptab->p_top);
	int8_t want = maps_needed(size + MAPBASE);
	uint8_t *ptr = (uint8_t *)udata.u_page;
	int i;

#ifdef CONFIG_VIDMAP8
	if (ptr[8] != PAGE_INVALID)
		vidmap_unmap();
#endif

	/* No change no work */
	if (want == have)
		return 0;

	/* If we are shrinking then free pages and propogate the
	   common page or invalid into the freed spaces */
	if (want < have) {
		uint8_t last = ptr[7 - LOBANK];
		ptr += want - 1;
		for (i = want; i < have; i++) {
			pfree[pfptr++] = *ptr;
			/* We might replicate the top page or we might
			   replicate the invalid - both work */
			*ptr++ = last;
		}
		program_vectors(&udata.u_page);
		return 0;
	}
#ifdef SWAPDEV
	/* Throw our toys out of our pram until we have enough room */
	while (want - have > pfptr)
		if (swapneeded(udata.u_ptab, 1) == NULL)
			return ENOMEM;
#else
	if (want - have > pfptr)	/* We have no swap so poof... */
		return ENOMEM;
#endif
	ptr += have - 1;
	for (i = have; i < want; i++)
		*ptr++ = pfree[--pfptr];
	program_vectors(&udata.u_page);
	return 0;
}

int pagemap_prepare(struct exec *hdr)
{
	/* TODO: add graphics reservation hint hooks */
	/* If it is relocatable load it at PROGLOAD */
	if (hdr->a_base == 0)
		hdr->a_base = PROGLOAD >> 8;
	/* If it doesn't care about the size then the size is all the
	   space we have */
	if (hdr->a_size == 0)
		hdr->a_size = (ramtop >> 8) - hdr->a_base;
	return 0;
}


usize_t pagemap_mem_used(void)
{
	return procmem - (pfptr << 3);
}

#ifdef SWAPDEV

/*
 *	Swap out the memory of a process to make room
 *	for something else. For bank8k do this as up to 9 operations and
 *	skip invalid pages.
 */

int swapout(ptptr p)
{
	uint16_t page = p->p_page;
	uint16_t blk;
	int16_t map;
	uint16_t base = SWAPBASE;
	uint16_t size = (0x2000 - SWAPBASE) >> 9;
	uint8_t i;
	uint8_t *pt = (uint8_t *)p->p_page;
#ifdef CONFIG_VIDMAP8
	uint8_t pv = pt[8];
#endif	

	if (!page)
		panic(PANIC_ALREADYSWAP);
#ifdef DEBUG
	kprintf("Swapping out %x (%x)\n", p, p->p_page);
#endif

	/* Are we out of swap ? */
	map = swapmap_alloc();
	if (map == -1)
		return ENOMEM;
	blk = map * SWAP_SIZE;
	/* Write the app (and possibly the uarea etc..) to disk */
	for (i = LOBANK; i < HIBANK; i++) {
		uint8_t pg = *pt++;
		if (i == HIBANK - 1)
			size = TOP_SIZE;
		else
			size = 0x10;
		if (pg != PAGE_INVALID) {
#ifdef CONFIG_VIDMAP8
			if (pg == PAGE_VIDEO)
				pg = pv;
#endif				
			swapwrite(SWAPDEV, blk, size << 9, base, pg);
		}
		base += 0x2000;
		base &= 0xE000;	/* Snap to bank alignment */
		blk += size;
	}
	pagemap_free(p);
	p->p_page = 0;
	p->p_page2 = map;

#ifdef DEBUG
	kprintf("%x: swapout done %d\n", p, p->p_page);
#endif
	return 0;
}

/*
 * Swap ourself in: must be on the swap stack when we do this
 */

void swapin(ptptr p, uint16_t map)
{
	uint16_t blk = map * SWAP_SIZE;
	uint16_t base = SWAPBASE;
	uint16_t size = (0x2000 - SWAPBASE) >> 9;
	uint8_t i;
	uint8_t *pt = (uint8_t *)p->p_page;
#ifdef CONFIG_VIDMAP8
	uint8_t pv = pt[8];
#endif	

#ifdef DEBUG
	kprintf("Swapin %x, %x\n", p, p->p_page);
#endif
	if (!p->p_page) {
		kprintf("%x: nopage!\n", p);
		return;
	}

	for (i = LOBANK; i < HIBANK; i++) {
		uint8_t pg = *pt++;
		if (i == HIBANK - 1)
			size = TOP_SIZE;
		else
			size = 0x10;	/* 8K */
		if (pg != PAGE_INVALID) {
#ifdef CONFIG_VIDMAP8		
			if (pg == PAGE_VIDEO)
				pg = pv;
#endif				
			swapread(SWAPDEV, blk, size << 9, base, pg);
		}
		base += 0x2000;
		base &= 0xE000;
		blk += size;
		/* FIXME: if we have a shared common then the size is not 0x10
		   for the last block */
	}
#ifdef DEBUG
	kprintf("%x: swapin done %d\n", p, p->p_page);
#endif
}

#endif

#endif
