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

/* This checks to see if a user-supplied address is legitimate */
usize_t valaddr(const uint8_t *base, usize_t size)
{
	if (base + size < base)
		size = MAXUSIZE - (usize_t)base + 1;
	if (!base || base < (const uint8_t *)DATABASE)
		size = 0;
	else if (base + size > (const uint8_t *)(size_t)udata.u_top)
		size = (uint8_t *)(size_t)udata.u_top - base;
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
}

/*
 *	Swap out the memory of a process to make room
 *	for something else
 */
int swapout_new(ptptr p, void *u)
{
	panic("swapout_new");

#if 0
	uint16_t page = p->p_page;
	uint16_t blk;
	int16_t map;

#ifdef DEBUG
	kprintf("Swapping out %x (%d)\n", p, p->p_pid);
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
	kprintf("%x: swapout done %d\n", p, p->p_page2);
#endif
	return 0;
#endif 
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
	panic("swapin");
#if 0
	uint16_t blk = map * SWAP_SIZE;

#ifdef DEBUG
	kprintf("Swapin %x (%d, %d)\n", p, p->p_page2, p->p_pid);
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
	kprintf("%x: swapin done %d\n", p, p->p_page2);
#endif
#endif
}

