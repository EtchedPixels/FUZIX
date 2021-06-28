/*
 *	The NC100 and NC200 banking we drive slightly differently in order
 *	to deal with graphical apps.	
 */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <exec.h>

#undef DEBUG

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

	/* Don't try and put the graphics page into free space */
	if (p->p_flags & PFL_GRAPHICS)
		i++;
	for (; i < 4; i++) {
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
 *	Use p-> not udata. The udata is not yet valid
 */
int pagemap_alloc(ptptr p)
{
	uint8_t *ptr = (uint8_t *) & p->p_page;
	int needed = maps_needed(p->p_top);
	int i = 0;

	if (p->p_flags & PFL_GRAPHICS)
		ptr[i++] = 0x43;

	if (needed - i > pfptr)	/* We have no swap so poof... */
		return ENOMEM;

	/* Pages in the low then repeat the top one */
	for (; i < needed; i++)
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

	/* Fix up mapping 0 */
	if (hdr->a_hints & HINT_GRAPHICS) {
		/* Graphics on - switch map 0 */
		if (ptr[0] != 0x43) {
			pfree[pfptr++] = ptr[0];
			ptr[0] = 0x43;
		}
	} else if (ptr[0] == 0x43) {
		/* Graphics off, get a new map 0 */
		if (pfptr == 0)
			return -ENOMEM;
		ptr[0] = pfree[--pfptr];
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
	/* Graphical apps get loaded at 0x4000 or higher so we can flip the
	   low page */
	if (hdr->a_hints & HINT_GRAPHICS) {
		if (hdr->a_base == 0)
			hdr->a_base = 0x40;
		else if (hdr->a_base < 0x40) {
			kprintf("low base %x\n", hdr->a_base);
			udata.u_error = ENOMEM;
			return -1;
		}
		p->p_flags |= PFL_GRAPHICS;
	} else
		p->p_flags &= ~PFL_GRAPHICS;
	
	/* If it is relocatable load it at PROGLOAD */
	if (hdr->a_base == 0)
		hdr->a_base = PROGLOAD >> 8;
	/* If it doesn't care about the size then the size is all the
	   space we have */
	if (hdr->a_size == 0)
		hdr->a_size = (DEFAULT_TOP >> 8) - hdr->a_base;
	kprintf("base %x size %x\n", hdr->a_base, hdr->a_size);
	return 0;
}

usize_t pagemap_mem_used(void)
{
	return procmem - (pfptr << 4);
}

