/*
 *	This module manages a system with fixed sized banking where all but one
 *	area of memory is pinned and the other area can be set to multiple
 *	different banks.
 *
 *	FFFF		+-----------------------------------+
 *              	|       Banked Common Memory        |
 *    MAP_SPLIT		+-----------------------------------+
 *	        	|          Banked Memory            |
 *      0000		+-----------------------------------+
 *
 *
 *	Other requirements:
 *	- 16bit address space (under 64K is fine, over is not) [Wants fixing]
 *
 *	Set:
 *	CONFIG_BANK_FIXED
 *	MAX_MAPS	to the number of banks available for user programs
 *	SWAPDEV		if using swap
 *	MAP_SPLIT	the split point between the two
 *
 *	Bank numbers must not include 0 (0 is taken as swapped)
 *
 *	This mapping scheme is mostly simple except that when we swap something out we have a problem
 *	as the common space we need to access is not the current one and we may have no way to map it
 *	anywhere except the top of memory. Instead we use the swap stack when appropriate and provide
 *	the platform specific code a 'swap out' method to call on the swap stack after it has mapped
 *	the victim.
 */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>

#undef DEBUG

#ifdef CONFIG_BANK_SPLIT

/*
 *	The page value is 16bit, but our internal map table is 8bit. There
 *	are cases where the caller wants to use the full 16bits and expand
 *	compact the representation. The default behaviour is a 1:1 map
 */

#ifndef MAP_TRANS_8TO16
# define MAP_TRANS_8TO16(M)	(M)
# define MAP_TRANS_16TO8(M)	(M)
#endif

/* Kernel is 0, apps 1,2,3 etc */
static uint8_t pfree[MAX_MAPS];
static uint_fast8_t pfptr = 0;
static uint_fast8_t pfmax;

void pagemap_add(uint8_t page)
{
	if (pfptr == MAX_MAPS)
		panic(PANIC_MAPOVER);
	pfree[pfptr++] = page;
	pfmax = pfptr;
}

void pagemap_free(ptptr p)
{
	if (p->p_page == 0)
		panic(PANIC_FREE0);
	pfree[pfptr++] = MAP_TRANS_16TO8(p->p_page);
}

int pagemap_alloc(ptptr p)
{
#ifdef SWAPDEV
	if (pfptr == 0) {
		swapneeded(p, 1);
	}
#endif
	if (pfptr == 0)
		return ENOMEM;
	p->p_page = MAP_TRANS_8TO16(pfree[--pfptr]);
	return 0;
}

/* Realloc is a no-op */
int pagemap_realloc(struct exec *hdr, uint16_t size)
{
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
	/* Check it fits - we can do this early for fixed banks */
	if (hdr->a_size > (MAP_SPLIT >> 8)) {
		udata.u_error = ENOMEM;
		return -1;
	}
	return 0;
}

/* We switch top and bottom so this is usually 64K but there are systems
   with interesting layours (eg riz180) where the 64K virtual map is not
   a 64K physical map. Therefore we must use PROC_SIZE */
usize_t pagemap_mem_used(void)
{
	return (pfmax - pfptr) * PROC_SIZE;
}

#ifdef SWAPDEV
/*
 *	Swap out the memory of a process to make room for something else.
 *	The calling platform code is required to map the victim process
 *	before we are called, and restore the correct state afterwards. It
 *	must also provide a suitable stack. As the swapper may be running on
 *	the swap stack already it must also deal with this case.
 */
int do_swapout(ptptr p)
{
	uint16_t page = p->p_page;
	uint16_t blk;
	int16_t map;

	if (!page)
		panic(PANIC_ALREADYSWAP);
	/* Are we out of swap ? */
	map = swapmap_alloc();
	if (map == -1)
		return ENOMEM;
#ifdef DEBUG
	kprintf("Swapping out %x (%d) to map %d\n", p, p->p_page, map);
#endif
	blk = map * SWAP_SIZE;
	/* Write the app (and possibly the uarea etc..) to disk */
	swapwrite(SWAPDEV, blk, SWAPTOP - SWAPBASE, SWAPBASE, p->p_page);
	pagemap_free(p);
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
void swapin(ptptr p, uint16_t map)
{
	uint16_t blk = map * SWAP_SIZE;

#ifdef DEBUG
	kprintf("Swapin %x, %d from map %d\n", p, p->p_page, map);
#endif
	if (!p->p_page) {
		kprintf("%x: nopage!\n", p);
		return;
	}
	swapread(SWAPDEV, blk, SWAPTOP - SWAPBASE, SWAPBASE, p->p_page);
#ifdef DEBUG
	kprintf("%x: swapin done %d\n", p, p->p_page);
#endif
}

#endif

#endif
