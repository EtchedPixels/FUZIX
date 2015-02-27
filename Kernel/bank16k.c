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
 */
#include <kernel.h>
#include <kdata.h>
#include <printf.h>

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
		panic("map over");
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
 */
int pagemap_alloc(ptptr p)
{
	uint8_t *ptr = (uint8_t *) & p->p_page;
	int needed = maps_needed(udata.u_top);
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

	while (i < 4) {
		ptr[i] = ptr[i - 1];
		i++;
	}
	return 0;
}

/*
 *	Reallocate the maps for a process
 */
int pagemap_realloc(uint16_t size)
{
	int have = maps_needed(udata.u_top);
	int want = maps_needed(size);
	uint8_t *ptr = (uint8_t *) & udata.u_page;
	int i;
	irqflags_t irq;

	/* If we are shrinking then free pages and propogate the
	   common page into the freed spaces */
	if (want == have)
		return 0;
	if (have > want) {
		for (i = want; i < have; i++) {
			pfree[pfptr++] = ptr[i];
			ptr[i] = ptr[3];
		}
		udata.u_ptab->p_page = udata.u_page;
		udata.u_ptab->p_page2 = udata.u_page2;
		return 0;
	}

	/* If we are adding then just insert the new pages, keeping the common
	   unchanged at the top */
	if (want - have > pfptr)
		return ENOMEM;
	/* We don't want to take an interrupt here while our page mappings are
	   incomplete. We may restore bogus mappings and then take a second IRQ
	   into hyperspace */
        irq = di();

	for (i = have; i < want; i++)
		ptr[i - 1] = pfree[--pfptr];
	/* Copy the updated allocation into the ptab */
	udata.u_ptab->p_page = udata.u_page;
	udata.u_ptab->p_page2 = udata.u_page2;
	/* Now fix the vectors up - they've potentially teleported up to 48K up
	   the user address space, we need to put a copy back in low memory before
	   we switch to this memory map */
	program_vectors(&udata.u_page);

	irqrestore(irq);
	return 0;
}

uint16_t pagemap_mem_used(void)
{
	return pfptr << 4;
}

#ifdef SWAPDEV

/*
 *	Swap out the memory of a process to make room
 *	for something else. For bank16k do this as four operations
 *	ready for when we pass page values not processes to the drivers
 */

int swapout(ptptr p)
{
	uint16_t page = p->p_page;
	uint16_t blk;
	uint16_t map;
	uint16_t base = SWAPBASE;
	uint16_t size = (0x4000 - SWAPBASE) >> 9;
	uint16_t i;

	swapproc = p;

	if (page)
		panic("process already swapped!\n");
#ifdef DEBUG
	kprintf("Swapping out %x (%d)\n", p, p->p_page);
#endif

	/* Are we out of swap ? */
	map = swapmap_alloc();
	if (map == 0)
		return ENOMEM;
	blk = map * SWAP_SIZE;
	/* Write the app (and possibly the uarea etc..) to disk */
	for (i = 0; i < 4; i ++) {
		swapwrite(SWAPDEV, blk, size, (uint8_t *)base);
		base += 0x4000;
		/* Last bank is determined by SWAP SIZE. We do the maths
		   in 512's (0x60 = 0xC000) */
		if (i == 3)
			size = SWAP_SIZE - 0x60;
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
 */
void swapin(ptptr p)
{
	uint16_t blk = p->p_page2 * SWAP_SIZE;
	uint16_t base = SWAPBASE;
	uint16_t size = (0x4000 - SWAPBASE) >> 9;
	uint16_t i;

#ifdef DEBUG
	kprintf("Swapin %x, %d\n", p, p->p_page);
#endif
	if (!p->p_page) {
		kprintf("%x: nopage!\n", p);
		return;
	}

	/* Return our swap */
	swapmap_add(p->p_page2);

	swapproc = p;		/* always ourself */
	for (i = 0; i < 4; i ++) {
		swapread(SWAPDEV, blk, size, (uint8_t *)base);
		base += 0x4000;
		/* Last bank is determined by SWAP SIZE. We do the maths
		   in 512's (0x60 = 0xC000) */
		if (i == 3)
			size = SWAP_SIZE - 0x60;
		else
			size = 0x20;	/* 16 K */
	}
#ifdef DEBUG
	kprintf("%x: swapin done %d\n", p, p->p_page);
#endif
}

#endif

#endif
