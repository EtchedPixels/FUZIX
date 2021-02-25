#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <exec.h>

/*
 *	This module manages a system with flexible 16K sized banks. It assumes
 *	that the udata and kernel common/stacks/other overheads are mapped at the
 *	bottom of available memory. (0 to PROGBASE - 1)
 *
 *	If the memory is mapped with the common at the top (16K banks for Z80
 *	for example) then use bank16k.c
 *
 *	Other requirements:
 *	- If you are using swap your swap driver must know how to remap and access
 *	  any process memory for the other processes
 *	- 16bit address space (under 64K is fine, over is not)
 *
 *	Set:
 *	CONFIG_BANK16_LOW
 *	MAX_MAPS	to the largest number of free pages there can be
 *	SWAPDEV		if using swap
 *
 *	Page numbers must not include 0 (0 is taken as swapped)
 *
 *	Swap needs debugging and fixes before it will be usable (review
 *	bank16k.c)
 */


#ifdef CONFIG_BANK16_LOW

#ifndef CONFIG_LOW_PAGE
#define SWAP_SIZE_EXCL_ZP	SWAP_SIZE
#endif

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
	uint16_t needed = top ? top - 1 : top;
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

	while (i < 4) {
		ptr[i] = ptr[i - 1];
		i++;
	}
	return 0;
}

/*
 *	Reallocate the maps for a process
 */
int pagemap_realloc(struct exec *hdr, uint16_t size)
{
	int have = maps_needed(udata.u_ptab->p_top);
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

        /* We have common low so we must only touch the higher pages. This is
           different from the high common case */
	for (i = have; i < want; i++)
		ptr[i] = pfree[--pfptr];
        while(i < 4) {
		ptr[i] = ptr[i-1];
		i++;
	}
	/* Copy the updated allocation into the ptab */
	udata.u_ptab->p_page = udata.u_page;
	udata.u_ptab->p_page2 = udata.u_page2;

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

uint16_t pagemap_mem_used(void)
{
	return procmem - (pfptr << 4);
}


#ifdef SWAPDEV

/*
 *	Swap out the memory of a process to make room
 *	for something else. For bank16k do this as four operations
 *	ready for when we pass page values not processes to the drivers
 *
 *	FIXME: We also need to swap out page 0/1 on 6502 (ZP and S) as well
 *	as looking at the best way to include the uarea in the swaps
 */

int swapout(ptptr p)
{
	uint16_t page = p->p_page;
	uint16_t blk;
	int16_t map;
	uint16_t base = SWAPBASE;
	uint16_t size = (0x4000 - SWAPBASE) >> 9;
	uint8_t *pt = (uint8_t *)&p->page;

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

#ifdef CONFIG_HAS_LOW_PAGE
	swapwrite(SWAPDEV, blk, 1, 0x0000);
	blk++;
#endif

	/* Write the app (and possibly the uarea etc..) to disk */
	for (i = 0; i < 4; i ++) {
		swapwrite(SWAPDEV, blk, size<<9, base, *pt++);
		base += 0x4000;
		/* Last bank is determined by SWAP SIZE. We do the maths
		   in 512's (0x60 = 0xC000) */
		if (i == 2)
			size = SWAP_SIZE_EXCL_ZP - 0x60;
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
void swapin(ptptr p, uint16_t map)
{
	uint16_t blk = map * SWAP_SIZE;
	uint16_t base = SWAPBASE;
	uint16_t size = (0x4000 - SWAPBASE) >> 9;
        uint16_t i;
	uint8_t *pt = (uint8_t *)&p->page;

#ifdef DEBUG
	kprintf("Swapin %x, %d\n", p, p->p_page);
#endif
	if (!p->p_page) {
		kprintf("%x: nopage!\n", p);
		return;
	}

	/* This may need other tweaks as its a special nasty case where
	   we don't want to overwrite the live stack but buffer and fix up
	   in tricks.s */
#ifdef CONFIG_HAS_LOW_PAGE
	swapread(SWAPDEV, blk, 512, 0x0000);
	blk++;
#endif

	for (i = 0; i < 4; i ++) {
		swapread(SWAPDEV, blk, size<<9, base, *pt++);
		base += 0x4000;
		/* Last bank is determined by SWAP SIZE. We do the maths
		   in 512's (0x60 = 0xC000) */
		if (i == 2)
			size = SWAP_SIZE_EXCL_ZP - 0x60;
		else
			size = 0x20;	/* 16 K */
	}
#ifdef DEBUG
	kprintf("%x: swapin done %d\n", p, p->p_page);
#endif
}
#endif

#endif
