/*
 *	The ZX Spectrum 128 has 8 banks all of which can appear in the top 16K
 *	but two of whom are permanently wired to appear at 0x4000 and 0x8000.
 *
 *	The Russian clones (Pentagon, Scorpion etc) generally also have a way
 *	to place bank 0 in 0000-3FFF
 *
 *	There is no way to switch the 4000-BFFF space so we have to block
 *	copy stuff around (gak).
 *
 *	For now we just handle the 128K spectrum. The pages are allocated as
 *	follows
 *
 *	0 Kernel code bank 1 at C000-FFFF 	(will need to change for
 *						 pentagon/scorpion)
 *	1 Kernel code bank 2 at C000-FFFF
 *	2 Hardwired at 0x8000-0xBFFF (User space)
 *	3 User space top 16K for process
 *	4 User space too 16K for other process
 *	5 Hardwired at 0x4000-0x7FFF (main screen - we don't use) Kernel data
 *	6 Exchanged with page 2 on task switches
 *	7 Hardwired for shadow screen (the one we use) at C000 for 6912 bytes
 *	We use this for video and code bank 3 at C000-FFFF 
 *
 *	The Pentagon goes up to 1MB, all in top 16K banks. Page 0 can be
 *	assigned to the low 16K space. The Scorpion is similar but they don't
 *	quite agree on which bits do what.
 *
 *	The current code just hands out 4 and 3 and relies upon knowing that
 *	you always exchange 6 and 2. That won't work once we expand to the
 *	better systems.
 *
 *	To confuse things further RAM banks 1/3/5/7 are slower (it's not quite
 *	the same on the clones). We might want to juggle a few things around
 *	for best speed.
 */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <exec.h>

#undef DEBUG

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
int pagemap_realloc(struct exec *hdr, usize_t size)
{
	udata.u_ptab->p_size = 32;
	return 0;
}

int pagemap_prepare(struct exec *hdr)
{
	/* If it is relocatable load it at PROGLOAD */
	if (hdr->a_base == 0)
		hdr->a_base = PROGLOAD >> 8;
	if (hdr->a_base != (PROGLOAD >> 8)) {
		udata.u_error = ENOEXEC;
		return -1;
	}
	/* If it doesn't care about the size then the size is all the
	   space we have */
	if (hdr->a_size == 0)
		hdr->a_size = (ramtop >> 8) - hdr->a_base;
	if (hdr->a_size > (MAP_SIZE >> 8))
		return -1;
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

	/* We never swap the live process so the second page is always
	   page 6 (This is not be true once we go past 128K but that has
	   its own bankbig.c) */
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
#ifdef DEBUG
		kprintf("Low page owned by %x\n", low_bank);
#endif
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
