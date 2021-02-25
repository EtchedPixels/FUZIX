/*
 *	This module manages a system with flexible 16K sized banks. It assumes
 *	that the udata and kernel common/stacks/other overheads are mapped at the
 *	very top of available memory. (PROGTOP to 0xFFFF)
 *
 *	If the memory is mapped with the common at the bottom (16K banks for 6502
 *	for example) then use bank16k_low.c
 *
 *	Other requirements:
 *	- If you are using swap your swap driver must know how to remap and access
 *	  any process memory for the other processes
 *	- 16bit address space (under 64K is fine, over is not)
 *
 *	Set:
 *	CONFIG_BANK16
 *	MAX_MAPS	to the largest number of free pages there can be
 *	SWAPDEV		if using swap
 *	PROGTOP		first byte above process space
 *
 *	Page numbers must not include 0 (0 is taken as swapped)
 *
 *	SWAPBASE must be in the low 16K bank
 */
#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <exec.h>

#undef DEBUG

#ifdef CONFIG_BANK16
/*
 * Map handling: We have flexible paging. Each map table consists of a set of pages
 * with the last page repeated to fill any holes.
 */

static unsigned char pfree[MAX_MAPS];
static unsigned char pfptr = 0;

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
 */
void pagemap_free(ptptr p)
{
	uint8_t *ptr = (uint8_t *) & p->p_page;
	uint8_t last = 0xff;
	int i;
	for (i = 0; i < 4; i++) {
		if (*ptr != last) {
			pfree[pfptr++] = *ptr;
			last = *ptr;
		}
		ptr++;
	}
}

static int maps_needed(uint16_t top)
{
	/* On many platforms if you touch this or PROGTOP you must
	   touch tricks.s */
	uint16_t needed = top + 0xFFFF - PROGTOP;
	/* Usually we have 0x1000 common - 1 for shift and inc */
	needed >>= 14;		/* in banks */
	needed++;		/* rounded */
	return needed;
}

/*
 *	Allocate the maps for this task post fork
 *	We have a hackish fix for init that would be nice
 *	resolve.
 *
 *	Use p-> not udata. The udata is not yet swapped in!
 */
static int pagemap_alloc2(ptptr p, uint8_t c)
{
	uint8_t *ptr = (uint8_t *) & p->p_page;
	int needed = maps_needed(p->p_top);
	int i;

	if (c)
		needed--;
#ifdef SWAPDEV
	/* Throw our toys out of our pram until we have enough room */
	while (needed > pfptr)
		if (swapneeded(p, 1) == NULL)
			return ENOMEM;
#else
	if (needed > pfptr)	/* We have no swap so poof... */
		return ENOMEM;
#endif

	/* Pages in the low then repeat the top one */
	for (i = 0; i < needed; i++)
		ptr[i] = pfree[--pfptr];

	if (!c)
		c = ptr[i - 1];
	while (i < 4) {
		ptr[i] = c;
		i++;
	}
	return 0;
}

int pagemap_alloc( ptptr p ){
	return pagemap_alloc2(p, 0);
}

/*
 *	Reallocate the maps for a process
 *
 *	FIXME: this is a quick hack for non split I/D old style chmem
 *
 *	FIXME: review swap case and ENOMEM
 */
int pagemap_realloc(struct exec *hdr, usize_t size)
{
	int8_t have = maps_needed(udata.u_ptab->p_top);
	int8_t want = maps_needed(size + MAPBASE);
	uint8_t *ptr = (uint8_t *) & udata.u_page;
	int8_t i;
	uint8_t update = 0;
	irqflags_t irq;

	/* If we are shrinking then free pages and propogate the
	   common page into the freed spaces */
	if (want == have)
		return 0;

	/* We don't want to take an interrupt here while our page mappings are
	   incomplete. We may restore bogus mappings and then take a second IRQ
	   into hyperspace */
	irq = __hard_di();

	if (have > want) {
		/* FIXME: swapout handling is needed ahead of this */
		for (i = want; i < have; i++) {
			pfree[pfptr++] = ptr[i - 1];
			ptr[i - 1] = ptr[3];
		}
		/* We collapsed top and bottom, so we need to sort our vectors
		   and common space out */
		if (want == 1)
			update = 1;
	} else if (want - have <= pfptr) {
		/* If we are adding then just insert the new pages, keeping the common
		   unchanged at the top */
		for (i = have; i < want; i++)
			ptr[i - 1] = pfree[--pfptr];
		update = 1;

	} else {
		__hard_irqrestore(irq);
		return ENOMEM;
	}
	/* Copy the updated allocation into the ptab */
	udata.u_ptab->p_page = udata.u_page;
	udata.u_ptab->p_page2 = udata.u_page2;
	/* Now fix the vectors up - they've potentially teleported up to 48K up
	   the user address space, we need to put a copy back in low memory before
	   we switch to this memory map */
	if (update)
		program_vectors(&udata.u_page);
	__hard_irqrestore(irq);
	return 0;
}

int pagemap_prepare(struct exec *hdr)
{
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
	return procmem - (pfptr << 4);
}

#ifdef SWAPDEV

/* Returns a page with common copied to it
   used by swapping 16k platforms to procure a common page.
   platforms will have to provide a void copy_common(uint8_t page)
   that copies the current common page to the specified page.

   FIXME: don't assume common always freed last. Probably means always
   calling copy_common, but this is swap in so it's already slow so this
   shouldn't be a big deal.
*/

uint8_t get_common(void)
{
	ptptr p = NULL;
	/* if current context is dead, then reuse it's common */
	if (udata.u_ptab->p_status == P_ZOMBIE ||
	    udata.u_ptab->p_status == P_EMPTY){
		return pfree[--pfptr];
	}
	/* otherwise get alloc a page and copy common to it */
	if (pfptr){
		int ret = pfree[--pfptr];
		copy_common(ret);
		return ret;
	}
	/* No free pages so swap something out */
	/* and return it's common */
	swapneeded(p, 0);
	return pfree[--pfptr];
}

/* Finish swapping in rest of task p, using page as it's common
   - called from swapping 16k platforms to finish the swapping
   processes, after starting it with get_common()

*/
void swap_finish(uint8_t page, ptptr p)
{
	uint16_t map = p->p_page2;
	pagemap_alloc2(p, page);
	swapper2(p, map);
}


/*
 *	Swap out the memory of a process to make room
 *	for something else. For bank16k do this as four operations
 *	ready for when we pass page values not processes to the drivers
 *
 *	FIXME: bank16k should only read/write out the banks that are in use
 */

int swapout(ptptr p)
{
	uint16_t page = p->p_page;
	uint16_t blk;
	int16_t map;
	uint16_t base = SWAPBASE;
	uint16_t size = (0x4000 - SWAPBASE) >> 9;
	uint16_t i;
	uint8_t *pt = (uint8_t *) & p->p_page;

	if (!page)
		panic(PANIC_ALREADYSWAP);
#ifdef DEBUG
	kprintf("Swapping out %x (%d)\n", p, p->p_page);
#endif

	/* Are we out of swap ? */
	map = swapmap_alloc();
	if (map == -1)
		return ENOMEM;
	blk = map * SWAP_SIZE;
	/* Write the app (and possibly the uarea etc..) to disk */
	for (i = 0; i < 4; i++) {
		swapwrite(SWAPDEV, blk, size << 9, base, *pt++);
		base += 0x4000;
		base &= 0xC000;	/* Snap to bank alignment */
		blk += size;
		/* Last bank is determined by SWAP SIZE. We do the maths
		   in 512's (0x60 = 0xC000) */
		if (i == 2)
			size = SWAP_SIZE + (SWAPBASE >> 9) - 0x60;
		else
			size = 0x20;
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
 *
 * FIXME: bank16k should only read/write out the banks that are in use
 */
void swapin(ptptr p, uint16_t map)
{
	uint16_t blk = map * SWAP_SIZE;
	uint16_t base = SWAPBASE;
	uint16_t size = (0x4000 - SWAPBASE) >> 9;
	uint16_t i;
	uint8_t *pt = (uint8_t *) & p->p_page;

#ifdef DEBUG
	kprintf("Swapin %x, %d\n", p, p->p_page);
#endif
	if (!p->p_page) {
		kprintf("%x: nopage!\n", p);
		return;
	}

	for (i = 0; i < 4; i++) {
		swapread(SWAPDEV, blk, size << 9, base, *pt++);
		base += 0x4000;
		base &= 0xC000;
		blk += size;
		/* Last bank is determined by SWAP SIZE. We do the maths
		   in 512's (0x60 = 0xC000) */
		if (i == 2)
			size = SWAP_SIZE + (SWAPBASE >> 9) - 0x60;
		else
			size = 0x20;	/* 16 K */
	}

#ifdef DEBUG
	kprintf("%x: swapin done %d\n", p, p->p_page);
#endif
}

#endif

#endif
