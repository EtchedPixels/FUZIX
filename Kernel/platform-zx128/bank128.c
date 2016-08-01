/*
 *	This is a modified version of the banker used for the ZX Spectrum
 *	128K. On this platform life is a bit complicated as the only page
 *	area we can bank is the top 16K. The 0x8000-0xBFFF area is switched
 *	by an exchange scheme and as a result the paging logic must know
 *	about this and hide it from the I/O subsystem.
 */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>

#define DEBUG

/* Kernel is 0, apps are 4 and 3 (top 16K). The live one also has 2 and
   the other has 6 */
static unsigned char pfree[MAX_MAPS] = { 4, 3 };
static unsigned char pfptr = 2;

extern ptptr low_bank;
extern void dup_low_page(void);

void pagemap_free(ptptr p)
{
	if (p->p_page == 0)
		panic("free0");
	pfree[pfptr++] = p->p_page;
	/* Have we dropped the low page owner, if so low page is free */
	if (low_bank == p)
		low_bank = NULL;
}

int pagemap_alloc(ptptr p)
{
#ifdef SWAPDEV
	if (pfptr == 0)
		swapneeded(p, 1);
#endif
	if (pfptr == 0)
		return ENOMEM;
	p->p_page = pfree[--pfptr];
	return 0;
}

/* Realloc is trivial - we can't do anything useful */
int pagemap_realloc(uint16_t size)
{
	if (size > MAP_SIZE)
		return ENOMEM;
	return 0;
}

uint16_t pagemap_mem_used(void)
{
	return (2 - pfptr) * 32;
}

/*
 *	Swap out the memory of a process to make room
 *	for something else.
 */
int swapout(ptptr p)
{
#ifndef SWAPDEV
	p; /* to shut the compiler */
#else
	uint16_t blk;
	uint16_t map;

	if (!p->p_page)
		panic("process already swapped!\n");
#ifdef DEBUG
	kprintf("Swapping out %x (%d)\n", p, p->p_page);
#endif

	/* We mever swap the live process so the second page is always
	   page 6 */
        if (low_bank == p)
		panic("swapout");

	/* Are we out of swap ? */
	map = swapmap_alloc();
	if (map == 0)
		return ENOMEM;
	blk = map * SWAP_SIZE;
	/* Write the top page and the included uarea stash to disk */
	swapwrite(SWAPDEV, blk, 0x4000, 0xC000, p->p_page);
	/* Write the alt bank via the top 16K window */
	swapwrite(SWAPDEV, blk + 0x20, 0x4000, 0xC000, 6);
	/* Release the mapping */
	pagemap_free(p);
	p->p_page = 0;
	p->p_page2 = map;
#ifdef DEBUG
	kprintf("%x: swapout done %d\n", p, p->p_page);
#endif
#endif
	return 0;
}

/*
 * Swap ourself in: must be on the swap stack when we do this. We always
 * do the swap in to the non exchange bank, as we know we will be running
 * next.
 */
void swapin(ptptr p, uint16_t map)
{
#ifndef SWAPDEV
	p;map; /* to shut the compiler */
#else
	uint16_t blk = map * SWAP_SIZE;

#ifdef DEBUG
	kprintf("Swapin %x, %d\n", p, p->p_page);
#endif
	if (!p->p_page) {
		kprintf("%x: nopage!\n", p);
		return;
	}
	/* Does another process own the low page. If so we need to
	   copy it over (we don't want to load and exchange as the
	   exchange is slower than a copy then loading */
	if (low_bank != NULL) {
		kprintf("Low page owned by %x\n", low_bank);
		dup_low_page();
	}
	/* Load the upper 16K back in including the udata cache */
	swapread(SWAPDEV, blk, 0x4000, 0xC000, p->p_page);
	/* Load the lower 16K either into the main lower bank (page 2) */
	swapread(SWAPDEV, blk + 0x20, 0x4000, 0xC000, 2);
	/* We now own the low page */
	low_bank = p;
#ifdef DEBUG
	kprintf("%x: swapin done %d\n", p, p->p_page);
#endif
#endif
}
