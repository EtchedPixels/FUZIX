/*
 *	This module provides bank support for the WDC65C816
 *
 *	The low 64K is assumed to contain the I/O space, some RAM used
 *	for stacks and DP areas. We don't make any direct attempt to use the
 *	rest of the low 64K, although on a box with lots of low RAM you could
 *	put the kernel there too.
 *
 *	Banks 1...n hold processes (each has 64K) or the kernel. We could but
 *	don't support split I/D (different code/data page) at this point.
 *
 *	Beyond that it's a normal banked platform. The oddities are that we
 *	have no common, and that we must also swap the stack separately
 *	from bank 0, as well as the program bank.
 *
 *	Note: The code assumes we have the normal user map of 0x0000-0xFBFF
 *	then 512 bytes of udata material, then a gap then stubs and that we
 *	swap the full 64K minus stubs/gap. If we extend udata then stubs will
 *	end up getting swapped in the final block but it's R/O so nobody
 *	should care.
 */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <exec.h>

#ifdef CONFIG_BANK_65C816

/* Kernel is 0, apps 1,2,3 etc */
static unsigned char pfree[MAX_MAPS];
static unsigned char pfptr = 0;
static unsigned char pfmax;

void pagemap_add(uint8_t page)
{
	pfree[pfptr++] = page;
	pfmax = pfptr;
}

void pagemap_free(ptptr p)
{
	if (p->p_page == 0)
		panic(PANIC_FREE0);
	pfree[pfptr++] = p->p_page;
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
	p->p_page = pfree[--pfptr];
	return 0;
}

/* Realloc is trivial - we can't do anything useful

   FIXME: when we redo the loader this will need to handle split I/D
   and new stack model */
int pagemap_realloc(struct exec *hdr, usize_t size)
{
	udata.u_ptab->p_size = 60;
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

usize_t pagemap_mem_used(void)
{
	return (pfmax - pfptr) * (MAP_SIZE >> 10);
}

#ifdef SWAPDEV
/*
 *	Swap out the memory of a process to make room
 *	for something else
 *
 *	FIXME: we can write out base - p_top, then the udata providing
 *	we also modify our read logic here as well
 */
int swapout(ptptr p)
{
	uint16_t page = p->p_page;
	uint16_t blk;
	int16_t map;

	if (!page)
		panic(PANIC_ALREADYSWAP);
#ifdef DEBUG
	kprintf("Swapping out %x (%d)\n", p, p->p_page);
#endif
	/* Are we out of swap ? */
	map = swapmap_alloc();
	if (map == -1)
		return ENOMEM;
	blk = map * SWAP_SIZE;
	/* Write the user CPU stack and DP to disk */
	swapwrite(SWAPDEV, blk, 512, (STACK_BANKOFF + 2 * (p - ptab)) << 8, 0);
	/* Write the process including DP and C stack, plus the udata cache */
	swapwrite(SWAPDEV, blk + 1, SWAPTOP - SWAPBASE, SWAPBASE, p->p_page);
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
	kprintf("Swapin %x, %d\n", p, p->p_page);
#endif
	if (!p->p_page) {
		kprintf("%x: nopage!\n", p);
		return;
	}
	/* Read the user stack and DP from disk */
	swapread(SWAPDEV, blk, 512, (STACK_BANKOFF + 2 * (p - ptab)) << 8, 0);
	/* Read the process back in */
	swapread(SWAPDEV, blk + 1, SWAPTOP - SWAPBASE, SWAPBASE, p->p_page);
#ifdef DEBUG
	kprintf("%x: swapin done %d\n", p, p->p_page);
#endif
}

#endif

#endif
