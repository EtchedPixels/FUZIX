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

/* This checks to see if a user-supplied address is legitimate */
usize_t valaddr(const uint8_t *base, usize_t size)
{
	if (base + size < base)
		size = MAXUSIZE - (usize_t)base + 1;
	if (!base || base < (const uint8_t *)DATABASE)
		size = 0;
	else if (base + size > (const uint8_t *)(size_t)udata.u_ptab->p_top)
		size = (uint8_t *)(size_t)udata.u_ptab->p_top - base;
	if (size == 0)
		udata.u_error = EFAULT;
	return size;
}

uaddr_t pagemap_base(void)
{
	return DATABASE;
}

void pagemap_free(ptptr p)
{
	p->p_page = 0;
}

int pagemap_alloc(ptptr p)
{
	p->p_page = 1;
	return 0;
}

usize_t pagemap_mem_used(void)
{
	return (DATALEN + CODELEN) >> 10;
}

void pagemap_init(void)
{
	for (int i=0; i<MAX_SWAPS; i++)
		swapmap_init(i);
}

static void swapinout(int blk,
	int (*readwrite)(uint16_t dev, blkno_t blkno, usize_t nbytes,
                    uaddr_t buf, uint16_t page))
{
	/* Read/write the udata block. */

	readwrite(SWAPDEV, blk, UDATA_BLKS<<BLKSHIFT, (uaddr_t)&udata, 1);

	/* Data area */

	/* Write out just 0..break  The stack is below break, so that gets written too. */
	readwrite(SWAPDEV, blk + UDATA_BLKS,
		(uaddr_t)alignup(udata.u_break - DATABASE, 1<<BLKSHIFT),
		DATABASE, 1);

	/* Code area */

	readwrite(SWAPDEV, blk + UDATA_BLKS + (DATALEN >> BLKSHIFT),
		(uaddr_t)alignup(udata.u_texttop - CODEBASE, 1<<BLKSHIFT), CODEBASE, 1);
}

/*
 *	Swap out the memory of a process to make room
 *	for something else
 */
int swapout(ptptr p)
{
#ifdef DEBUG
	kprintf("Swapping out %x (%d)\n", p, p->p_pid);
#endif
	uint16_t page = p->p_page;
	if (!page)
		panic(PANIC_ALREADYSWAP);
	/* Are we out of swap ? */
	int map = swapmap_alloc();
	if (map == -1)
		return ENOMEM;
	int blk = map * SWAP_SIZE;

	swapinout(blk, swapwrite);

	p->p_page = 0;
	p->p_page2 = map;
#ifdef DEBUG
	kprintf("%x: swapout done %d\n", p, p->p_page2);
#endif
	return 0;
}

/*
 * Swap ourself in: must be on the swap stack when we do this
 */
void swapin(ptptr p, uint16_t map)
{
	/* Does the current process need swapping out? */

	int blk = map * SWAP_SIZE;

#ifdef DEBUG
	kprintf("Swapin %x (%d, %d)\n", p, p->p_page2, p->p_pid);
#endif
	if (!p->p_page) {
		kprintf("%x: nopage!\n", p);
		return;
	}

	swapinout(blk, swapread);

#ifdef DEBUG
	kprintf("%x: swapin done %d\n", p, p->p_page2);
#endif
}

// vim: sw=4 ts=4 et:

