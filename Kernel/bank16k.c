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
	/* FIXME: unhardwire the 0x1000 common assumption ! */
	uint16_t needed = top + 0xFFF;	/* 0x1000 common - 1 for shift and inc */
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
	int want = maps_needed((uint16_t)PROGBASE + size);
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

#endif
