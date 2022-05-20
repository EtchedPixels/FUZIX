/*
 *	The Centurion has a max of 256K of DRAM that is mappable via
 *	a 256 byte fast SRAM MMU in 2K pages. Each MMU bank (there are 8)
 *	has a 32 byte table entry. I/O is at the top of the 256K address space
 *
 *	As far as we know there is no "null" entry or fault mechanism.
 *
 *	Current mapping
 *	0000-00FF		Registers
 *	0100-E7FF		Application/Kernel
 *	E800-EFFF		Common
 *	F000-FBFF		I/O	}	may be able to map out
 *	FC00-FFFF		BootROM	}	for user ?
 */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>

#define PTSIZE		32
#define MAX_MAPS	256

#define PAGE_INVALID	0

uint8_t pmap[PTABSIZE][PTSIZE];

static uint8_t pfree[MAX_MAPS];
static unsigned pfptr;

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
	uint8_t *pt = (uint8_t *)p->p_page;
	uint8_t *e = (uint8_t *)p->p_page + 29;
	uint8_t last = PAGE_INVALID;

	while(pt < e) {
		if (*pt != last) {
			last = *pt;
			pfree[pfptr++] = *pt;
		}
		++pt;
	}
}

/* Make sure all the maps contain the common and device I/O. We can take
   the device I/O out of user later */
void map_init(void)
{
	uint8_t i = 0;
	while(i < PTABSIZE) {
		pmap[i][29] = 0x1D;
		pmap[i][30] = 0x7E;
		pmap[i][31] = 0x7F;
		++i;
	}
}

/*
 *	We are passed the number of bytes that are needed for this
 *	process. On top of that we require the extra space between
 *	the top of space and PROGTOP which is reserved for the common
 *	space and udata. If you have no need for this then it'll come out
 *	as size - 1, which is correct.
 *
 *	We calculate how many banks you need from address 0
 */
static int maps_needed(uint16_t top)
{
	/* On many platforms if you touch this or PROGTOP you must
	   touch tricks.s */
	/* FIXME: calc this allowing for the I/O pages */
	uint16_t needed = top + 0xFFFF - PROGTOP;
	/* Usually we have 0x1000 common - 1 for shift and inc */
	needed >>= 11;		/* in banks */
	++needed;		/* rounded */
	return needed;
}

/*
 *	Allocate the maps for this task post fork
 *	We have a hackish fix for init that would be nice
 *	resolve.
 *
 *	Use p-> not udata. The udata is not yet swapped in!
 */
int pagemap_alloc(ptptr p)
{
	uint8_t *ptr;
	int needed = maps_needed(p->p_top);
	int i;

	/* Cheapest way to keep it non zero */
	ptr = pmap[p - ptab];
	p->p_page = (uint16_t)ptr;

#ifdef SWAPDEV
	/* Throw our toys out of our pram until we have enough room */
	while (needed > pfptr)
		if (swapneeded(p, 1) == NULL)
			return ENOMEM;
#else
	if (needed > pfptr)	/* We have no swap so poof... */
		return ENOMEM;
#endif

	/* Allocate the pages upwards */
	for (i = 0; i < needed; i++)
		*ptr++ = pfree[--pfptr];
	/*
	 * 29 is the common, 30 and 31 are I/O
	 */
	while (i++ < 29) {
		*ptr = ptr[-1];
		ptr++;
	}
	
	return 0;
}

/*
 *	Reallocate the maps for a process
 *
 *	Currently only called on execve and we rely on this in the
 *	implementation here.
 */
int pagemap_realloc(struct exec *hdr, usize_t size)
{
	int8_t have = maps_needed(udata.u_top);
	int8_t want = maps_needed(size + MAPBASE);
	uint8_t *ptr = (uint8_t *)udata.u_page;
	int i;

	/* No change no work */
	if (want == have)
		return 0;

	/* If we are shrinking then free pages and propogate the
	   common page or invalid into the freed spaces */
	if (want < have) {
		uint8_t last = ptr[28];
		ptr += want - 1;
		for (i = want; i < have; i++) {
			pfree[pfptr++] = *ptr;
			*ptr++ = last;
		}
		program_vectors(&udata.u_page);
		return 0;
	}
#ifdef SWAPDEV
	/* Throw our toys out of our pram until we have enough room */
	while (want - have > pfptr)
		if (swapneeded(udata.u_ptab, 1) == NULL)
			return ENOMEM;
#else
	if (want - have > pfptr)	/* We have no swap so poof... */
		return ENOMEM;
#endif
	ptr += have - 1;
	for (i = have; i < want; i++)
		*ptr++ = pfree[--pfptr];
	program_vectors(&udata.u_page);
	return 0;
}

int pagemap_prepare(struct exec *hdr)
{
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
	return 0;
}

usize_t pagemap_mem_used(void)
{
	return procmem - (pfptr << 1);
}

#ifdef SWAPDEV

/*
 *	Swap out the memory of a process to make room
 *	for something else. For bank8k do this as up to 9 operations and
 *	skip invalid pages.
 *
 *	TODO convert to 2K pages
 */

int swapout(ptptr p)
{
	uint16_t page = p->p_page;
	uint16_t blk;
	int16_t map;
	uint16_t base = SWAPBASE;
	uint16_t size = (0x2000 - SWAPBASE) >> 9;
	uint8_t i;
	uint8_t *pt = (uint8_t *)p->p_page;

	if (!page)
		panic(PANIC_ALREADYSWAP);
#ifdef DEBUG
	kprintf("Swapping out %x (%x)\n", p, p->p_page);
#endif

	/* Are we out of swap ? */
	map = swapmap_alloc();
	if (map == -1)
		return ENOMEM;
	blk = map * SWAP_SIZE;
	/* Write the app (and possibly the uarea etc..) to disk */
	for (i = LOBANK; i < HIBANK; i++) {
		uint8_t pg = *pt++;
		if (i == HIBANK - 1)
			size = TOP_SIZE;
		else
			size = 0x10;
		if (pg != PAGE_INVALID) {
			swapwrite(SWAPDEV, blk, size << 9, base, pg);
		}
		base += 0x2000;
		base &= 0xE000;	/* Snap to bank alignment */
		blk += size;
	}
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
	uint16_t base = SWAPBASE;
	uint16_t size = (0x2000 - SWAPBASE) >> 9;
	uint8_t i;
	uint8_t *pt = (uint8_t *)p->p_page;

#ifdef DEBUG
	kprintf("Swapin %x, %x\n", p, p->p_page);
#endif
	if (!p->p_page) {
		kprintf("%x: nopage!\n", p);
		return;
	}

	for (i = LOBANK; i < HIBANK; i++) {
		uint8_t pg = *pt++;
		if (i == HIBANK - 1)
			size = TOP_SIZE;
		else
			size = 0x10;	/* 8K */
		if (pg != PAGE_INVALID) {
			swapread(SWAPDEV, blk, size << 9, base, pg);
		}
		base += 0x2000;
		base &= 0xE000;
		blk += size;
		/* FIXME: if we have a shared common then the size is not 0x10
		   for the last block */
	}
#ifdef DEBUG
	kprintf("%x: swapin done %d\n", p, p->p_page);
#endif
}

#endif
