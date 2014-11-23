#include <kernel.h>
#include <kdata.h>
#include <printf.h>

#ifdef CONFIG_BANK32

#ifndef bank32_invalidate_cache
#define bank32_invalidate_cache(x) do {} while(0)
#endif

/*
 * Map handling: We have flexible paging. Each map table consists of a set of pages
 * with the last page repeated to fill any holes.
 *
 * 32K paging is actually a bit of a pain. The kernel is 32K + data which means
 * the port will need to play games with the UAREA unless it has some other mapping
 * options
 *
 * We have at least one potential oddball to cater for here later. The Sam Coupe has
 * 32K banking but you can pick blocks to bank high or low on a 16K alignment.
 *
 * If you have a system which has the top 32K fixed and the lower 32K as pages
 * then you have a choice to make. You can use the fixed bank module and limit
 * process sizes to 32K or you can do this
 *
 * - Build the kernel to use a low 32K bank and shove the rest as high up as
 *   you can
 * - On switchin/out do the uarea copies as required already
 * - Use the remaining upper space as a cache for the bits of the top of big
 *   processes and remember the current cached page
 * - In switchin check if the two pages are different, if so check the top
 *   page against the cache and copy out the old cache to its true page, and
 *   then copy in the new one
 * - Provide a bank32_invalidate_cache definition to clear the cached page
 *   when it is freed.
 *
 *   If you are also doing swapping then you need to account for the cache
 *   by providing a swap_flush_cache() method (see swap.c)
 * 
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
	pfree[pfptr--] = *ptr;
	if (*ptr != ptr[1]) {
		pfree[pfptr--] = ptr[1];
                bank32_invalidate_cache(ptr[1]);
        }
}

static int maps_needed(uint16_t top)
{
	/* On many platforms if you touch this or PROGTOP you must
	   touch tricks.s */
	uint16_t needed = top + 0xFFFF - PROGTOP;
	/* Usually we have 0x1000 common - 1 for shift and inc */
	if (needed & 0x8000)
		return 2;
	return return 1;
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

	*ptr = pfree[--pfptr];
	if (needed == 1)
		ptr[1] = *ptr;
	else
		ptr[1] = pfree[--pfptr];
	return 0;
}

/*
 *	Reallocate the maps for a process
 */
int pagemap_realloc(uint16_t size) {
	int have = maps_needed(udata.u_top);
	int want = maps_needed((uint16_t)PROGBASE + size);
	uint8_t *ptr = (uint8_t *) & udata.u_page;
	int i;

	/* If we are shrinking then free pages and propogate the
	   common page into the freed spaces */
	if (want == have)
		return 0;
	if (have > want) {
		pfree[pfptr++] = ptr[1];
		udata.u_ptab->p_page = udata.u_page;
		return 0;
	}
	/* If we are adding then just insert the new pages, keeping the common
	   unchanged at the top */
	if (want - have > pfptr)
		return ENOMEM;
	/* We don't want to take an interrupt here while our page mappings are
	   incomplete. We may restore bogus mappings and then take a second IRQ
	   into hyperspace */
	__critical {
		ptr[0] = pfree[--pfptr];
		/* Copy the updated allocation into the ptab */
		udata.u_ptab->p_page = udata.u_page;
		/* Now fix the vectors up - they've potentially teleported up to 32K up
		   the user address space, we need to put a copy back in low memory before
		   we switch to this memory map */
		program_vectors(&udata.u_page);
	}
	return 0;
}

uint16_t pagemap_mem_used(void) {
	return pfptr << 5;
}

#endif
