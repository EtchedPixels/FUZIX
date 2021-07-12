/*
 *	The NC100 and NC200 banking we drive slightly differently in order
 *	to deal with graphical apps.
 *
 *	We switch between stanard bank16k mappings and a mapping of
 *
 *	app page 0
 *	app page 1
 *	0x43 (video memory)
 *	kernel common for this app
 */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <exec.h>

#undef DEBUG

#define BANK_VIDEO	0x43

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
	int i = 0;

	for (; i < 4; i++) {
		/* Don't try and put the graphics page into free space */
		if (i == 2 && p->p_flags & PFL_GRAPHICS)
			continue;
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
	uint16_t needed;

	needed = top + 0xFFFF - PROGTOP;
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
 *	Use p-> not udata. The udata is not yet valid
 */
int pagemap_alloc(ptptr p)
{
	uint8_t *ptr = (uint8_t *) & p->p_page;
	int needed = maps_needed(p->p_top);
	uint8_t i;

	if (p->p_flags & PFL_GRAPHICS)
		needed = 3;

	if (needed > pfptr)	/* We have no swap so poof... */
		return ENOMEM;

	if (p->p_flags & PFL_GRAPHICS) {
		/* Special graphics map */
		*ptr++ = pfree[--pfptr];
		*ptr++ = pfree[--pfptr];
		*ptr++ = BANK_VIDEO;
		*ptr = pfree[--pfptr];
		return 0;
	}
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
 *
 *	This is a shade trickier because we may have been a graphical using
 *	app than then ran something.
 */
int pagemap_realloc(struct exec *hdr, usize_t size)
{
	int8_t have = maps_needed(udata.u_top);
	int8_t want = maps_needed(size + MAPBASE);
	uint8_t *ptr = (uint8_t *) & udata.u_page;
	int8_t i;
	uint8_t update = 0;
	irqflags_t irq;

	/* Transitioning from graphics, swap the graphics page for a common
	   copy */
	if (hdr->a_hints & HINT_GRAPHICS) {
		if (ptr[2] != BANK_VIDEO) {
			want = 3;
			if (want - have > pfptr)
				return ENOMEM;
			/* Free or add map pages as needed */
			if (have == 4)
				pfree[--pfptr] = ptr[2];
			if (have == 1)
				ptr[0] = pfree[pfptr++];
			if (have < 3)
				ptr[1] = pfree[pfptr++];
			ptr[2] = BANK_VIDEO;
		}
		return 0;
	}
	else if (ptr[2] == BANK_VIDEO) {
		/* Graphics off, fix up the map */
		ptr[2] = ptr[3];
		have = 3;
	}
			
	/* If we are shrinking then free pages and propogate the
	   common page into the freed spaces */
	if (want == have)
		return 0;

	/* We don't want to take an interrupt here while our page mappings are
	   incomplete. We may restore bogus mappings and then take a second IRQ
	   into hyperspace */
	irq = __hard_di();

	if (have > want) {
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
	ptptr p = udata.u_ptab;

	
	/* If it is relocatable load it at PROGLOAD */
	if (hdr->a_base == 0)
		hdr->a_base = PROGLOAD >> 8;
	if (hdr->a_base != (PROGLOAD >> 8)) {
		udata.u_error = ENOEXEC;
		return -1;
	}
	/* If it doesn't care about the size then the size is all the
	   space we have */

	if (hdr->a_hints & HINT_GRAPHICS) {
		p->p_flags |= PFL_GRAPHICS;
		/* Has to fit below the video mapping */
		if (hdr->a_size == 0)
			hdr->a_size = 0x80 - hdr->a_base;
	} else {
		p->p_flags &= ~PFL_GRAPHICS;
		if (hdr->a_size == 0)
			hdr->a_size = (DEFAULT_TOP >> 8) - hdr->a_base;
	}
	return 0;
}

usize_t pagemap_mem_used(void)
{
	return procmem - (pfptr << 4);
}

