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
 *	Other requirements:
 *	- 16bit address space (FIXME: should be made 32bit clean)
 *
 *	Set:
 *	CONFIG_SWAP_ONLY
 *
 *	Zero is used as swapped, 1 is used as in memory.
 */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>

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

int pagemap_realloc(uint16_t size)
{
  if (size >= ramtop)
    return ENOMEM;
  return 0;
}

uint16_t pagemap_mem_used(void)
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
int swapout(ptptr p)
{
	uint16_t page = p->p_page;
	uint16_t blk;
	uint16_t map;

	swapproc = p;

	if (!page)
		panic("process already swapped!\n");
#ifdef DEBUG
	kprintf("Swapping out %x (%d)\n", p, p->p_page);
#endif
	/* Are we out of swap ? */
	map = swapmap_alloc();
	if (map == 0)
		return ENOMEM;
	blk = map * SWAP_SIZE;
	/* Write the app (and possibly the uarea etc..) to disk */
	swapwrite(SWAPDEV, blk, SWAPTOP - SWAPBASE,
		  SWAPBASE);
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
void swapin(ptptr p)
{
	uint16_t blk = p->p_page2 * SWAP_SIZE;

#ifdef DEBUG
	kprintf("Swapin %x, %d\n", p, p->p_page);
#endif
	if (!p->p_page) {
		kprintf("%x: nopage!\n", p);
		return;
	}

	/* Return our swap */
	swapmap_add(p->p_page2);

	swapproc = p;		/* always ourself */
	swapread(SWAPDEV, blk, SWAPTOP - SWAPBASE,
		 SWAPBASE);
#ifdef DEBUG
	kprintf("%x: swapin done %d\n", p, p->p_page);
#endif
}

#endif

