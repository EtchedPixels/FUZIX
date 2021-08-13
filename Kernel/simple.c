/*
 *	This module manages a system with no user banking.
 *
 *	User memory lies between PROGBASE and PROGTOP which does not overlap
 *	common memory. The kernel may be banked over user memory areas if needed.
 *
 *	All task switching occurs by swapping the existing process out to storage
 *	and reading in the new one. This can be done for either single tasking or
 *	(with a hard disk) multitasking.
 *
 *	Set:
 *	CONFIG_SWAP_ONLY
 *
 *	Zero is used as swapped, 1 is used as in memory.
 *
 *	TODO:
 *	Write a simple-big module for systems like 68000 where it may be worth
 *	writing out only the used areas of memory and fast zeroing the rest
 *	on switches. It makes no sense to support it here because to do that
 *	well we want to support a simple first fit allocator for swap ranges
 *	so our swap isn't huge and empty.
 *
 *	The fact we don't do this for 8bit systems may seem weird, but on most
 *	of the systems supported a contiguous series of disk reads of 512
 *	byte blocks isn't *that* much slower than a memory zero. (inir + sector
 *	setup versus ldir on Z80 for example)
 */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <exec.h>

#undef DEBUG

#ifdef CONFIG_SWAP_ONLY

void pagemap_free(ptptr p)
{
	p->p_page = 0;
}

int pagemap_alloc(ptptr p)
{
	p->p_page = 1;
	return 0;
}

/* FIXME: update once we have the new mm logic in place */
int pagemap_realloc(struct exec *hdr, usize_t size)
{
	if (size > ramtop - PROGBASE)
		return ENOMEM;
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
	return (PROGTOP - PROGBASE) >> 10;
}

void pagemap_init(void)
{
}

/*
 *	Swap out the memory of a process to make room
 *	for something else
 */
int swapout_new(ptptr p, void *u)
{
	uint16_t page = p->p_page;
	uint16_t blk;
	int16_t map;

#ifdef DEBUG
	kprintf("Swapping out %p (%d)\n", p, p->p_pid);
#endif
	if (!page)
		panic(PANIC_ALREADYSWAP);
	/* Are we out of swap ? */
	map = swapmap_alloc();
	if (map == -1)
		return ENOMEM;
	blk = map * SWAP_SIZE;
	/* Write the app (and uarea etc..) to disk */
#ifdef CONFIG_SPLIT_UDATA
	/* Write the udata block as kernel. */
	udata.u_dptr = u;
	udata.u_block = blk;
	udata.u_nblock = UDATA_SIZE >> BLKSHIFT;	/* 1 */
	((*dev_tab[major(SWAPDEV)].dev_write) (minor(SWAPDEV), 0, 0));
	/* Use the standard swapwrite helper for the rest */
	swapwrite(SWAPDEV, blk + UDATA_BLKS, SWAPTOP - SWAPBASE,
		  SWAPBASE, 1);
#else
#error "Not supported"
#endif
	p->p_page = 0;
	p->p_page2 = map;
#ifdef DEBUG
	kprintf("%p: swapout done %d\n", p, p->p_page2);
#endif
	return 0;
}

int swapout(ptptr p)
{
	return swapout_new(p, &udata);
}

/*
 * Swap ourself in: must be on the swap stack when we do this
 */
void swapin(ptptr p, uint16_t map)
{
	uint16_t blk = map * SWAP_SIZE;

#ifdef DEBUG
	kprintf("Swapin %p (%d, %d)\n", p, p->p_page2, p->p_pid);
#endif
	if (!p->p_page) {
		kprintf("%x: nopage!\n", p);
		return;
	}

	/* The udata might not be in common space read it as kernel mapped */
	udata.u_dptr = (uint8_t *)&udata;
	udata.u_block = blk;
	udata.u_nblock = UDATA_SIZE >> BLKSHIFT;
	/* The driver will use the data in udata before it writes over it */
	((*dev_tab[major(SWAPDEV)].dev_read) (minor(SWAPDEV), 0, 0));
	swapread(SWAPDEV, blk + UDATA_BLKS, SWAPTOP - SWAPBASE,
		 SWAPBASE, 1);

#ifdef DEBUG
	kprintf("%p: swapin done %d\n", p, p->p_page2);
#endif
}

void swapinout(ptptr p)
{
	/* If there's an existing process in memory, swap it out; then swap in the
	 * new process. */
	if (udata.u_ptab->p_page)
		swapout(udata.u_ptab);
	swapper(p);
	p->p_page = 1;
}

#endif

