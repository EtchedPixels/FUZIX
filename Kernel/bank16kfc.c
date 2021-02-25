/*
 *	A variant of the 16K banks model where we pin the top 16K bank as a
 *	common like bankfixed but dynamically allocate blocks below that.
 *
 *	Useful if you have a platform with more limited memory and don't want
 *	to waste chunks with repeated common or high code copies.
 *
 *	Other requirements:
 *	- 16bit address space (under 64K is fine, over is not)
 *
 *	Set:
 *	CONFIG_BANK16FC
 *	MAX_MAPS	to the largest number of free pages there can be
 *	SWAPDEV		if using swap
 *
 *	Page numbers must not include 0 (0 is taken as swapped)
 *
 *	SWAPBASE must be in the low 16K bank
 *	PROGTOP must be 0xBE00 (We assume 4 x 16K banks)
 *		and 0xBE00-0xBFFF are the udata stash.
 */
#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <exec.h>

#undef DEBUG

#ifdef CONFIG_BANK16FC
/*
 * Map handling: We have flexible paging. Each map table consists of a set of pages
 * with the last page repeated to fill any holes.
 *
 * FIXME: We need to track requested stack max in a future update and allocate
 * accordingly.
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
	for (i = 0; i < 3; i++) {
		if (*ptr != last) {
			pfree[pfptr++] = *ptr;
			last = *ptr;
		}
		ptr++;
	}
}

static int maps_needed(uint16_t top)
{
	/* Allow 512 bytes for the udata stash at the top */
	uint16_t needed = top + sizeof(struct u_block) - PROGBASE - 1;
	needed >>= 14;		/* in banks */
	needed++;		/* rounded */
	return needed;
}

/*
 *	Allocate the maps for this task post fork
 *
 *	Use p-> not udata. The udata is not yet swapped in!
 */
int pagemap_alloc(ptptr p)
{
	uint8_t *ptr = (uint8_t *) & p->p_page;
	int needed = maps_needed(p->p_top);
	int i;

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
	for (i = needed; i < 3; i++)
		ptr[i] = ptr[i - 1];
	return 0;
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
	int8_t needed = want - have;

	/* No size change - no work required: usual path */
	if (want == have)
		return 0;

#ifdef SWAPDEV
	/* Throw our toys out of our pram until we have enough room */
	if (needed > 0) {
		while ((uint8_t)needed > pfptr)
			if (swapneeded(udata.u_ptab, 1) == NULL)
				return ENOMEM;
	}
#else
	if (needed > pfptr)	/* We have no swap so poof... */
		return ENOMEM;
#endif

	/* We don't want to take an interrupt here while our page mappings are
	   incomplete. We may restore bogus mappings and then take a second IRQ
	   into hyperspace */
	irq = __hard_di();

	if (have > want) {
		/* FIXME: swapout handling is needed ahead of this */
		for (i = want; i < have; i++) {
			pfree[pfptr++] = ptr[i - 1];
			ptr[i - 1] = ptr[2];
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
	/* Now fix the vectors up - they've potentially teleported up to 32K up
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
	kprintf("Swapping out %x (%x%x)\n", p, p->p_page, p->p_page2);
#endif

	/* Are we out of swap ? */
	map = swapmap_alloc();
	if (map == -1)
		return ENOMEM;
	blk = map * SWAP_SIZE;
	/* Write the app and udata stash to disk */
	for (i = 0; i < 3; i++) {
#ifdef DEBUG
		kprintf("%d: swapwrite block %d, size %x, base %x\n",
			p->p_pid, blk, size, base);
#endif
		swapwrite(SWAPDEV, blk, size << 9, base, *pt++);
		base += 0x4000;
		base &= 0xC000;	/* Snap to bank alignment */
		blk += size;
		size = 0x20;
	}
	pagemap_free(p);
	p->p_page = 0;
	/* Eventually we need to write the bitmask of pages actually
	   allocated (and thus swapped) in bits 8+ of map, so we can
	   recover it nicely on swap in */
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
	kprintf("Swapin %x, %x%x\n", p, p->p_page, p->p_page2);
#endif
	if (!p->p_page) {
		kprintf("%x: nopage!\n", p);
		return;
	}

	for (i = 0; i < 3; i++) {
#ifdef DEBUG
		kprintf("%d: swapread block %d, size %x, base %x\n",
			p->p_pid, blk, size, base);
#endif
		swapread(SWAPDEV, blk, size << 9, base, *pt++);
		base += 0x4000;
		base &= 0xC000;
		blk += size;
		size = 0x20;	/* 16 K */
	}

#ifdef DEBUG
	kprintf("%x: swapin done %d\n", p, p->p_page);
#endif
}

#endif

#endif
