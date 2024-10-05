/*
 *	Banking logic for ZX style machines with more than the 128K base
 *	memory.
 *
 *	We manage this as a pair of 16K banks allocated per process. To avoid
 *	the nasty memory swapping hit we don't swap the 32-48K area but
 *	copy in and out of it
 *
 *	We keep two page pointers, one in p->p_page and one in p->p_page2.
 *	The first holds our 0x8000-BFFF pages the second our upper page and
 *	udata stash.
 *
 *	The fact we load into different pages directly means the swap_map
 *	define isn't helpful. Instead as it's for a specific class of machine
 *	we just hardcode the addresses.
 */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <exec.h>

extern uint8_t switchedbank, switchedwb;

#undef DEBUG

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
	pfree[pfptr++] = p->p_page2;
	if (p->p_page == switchedbank) {
		switchedbank = 0;
		switchedwb = 0;
	}
}

int pagemap_alloc(ptptr p)
{
        /* We allocate in pairs so the end result is that a swap out makes
           room for a swap in. If we get clever we'll need a loop here */
#ifdef SWAPDEV
	if (pfptr < 2) {
		swapneeded(p, 1);
	}
#endif
	if (pfptr < 2)
		return ENOMEM;
	p->p_page = pfree[--pfptr];
	p->p_page2 = pfree[--pfptr];
	return 0;
}

/* Realloc is trivial - we can't do anything useful */
/* FIXME: update when new model is ready */
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
	/* If it doesn't care about the size then the size is all the
	   space we have */
	if (hdr->a_size == 0)
		hdr->a_size = (ramtop >> 8) - hdr->a_base;
	/* Check it fits - we can do this early for fixed banks */
	if (hdr->a_size > (MAP_SIZE >> 8))
		return -1;
	return 0;
}


usize_t pagemap_mem_used(void)
{
	return (pfmax - pfptr) * (16384 >> 10);
}

#ifdef SWAPDEV
/*
 *	Swap out the memory of a process to make room
 *	for something else. We never swap out the live process.
 */
int swapout(ptptr p)
{
	uint16_t page = p->p_page;
	uint16_t blk;
	uint16_t map;

	if (!page)
		panic(PANIC_ALREADYSWAP);
	/* Are we out of swap ? */
	map = swapmap_alloc();
	if (map == 0)
		return ENOMEM;
#ifdef DEBUG
	kprintf("Swapping out %x (%d) to map %d\n", p, p->p_page, map);
#endif
	blk = map * SWAP_SIZE;
	/* Write the app and the uarea to disk */
	swapwrite(SWAPDEV, blk, 0x4000, 0xC000, p->p_page);
	swapwrite(SWAPDEV, blk + 0x20, 0x4000, 0xC000, p->p_page2);
	pagemap_free(p);
	p->p_page = 0;
	p->p_page2 = map;
#ifdef DEBUG
	kprintf("%x: swapout done %d\n", p, p->p_page);
#endif
	return 0;
}

/*
 * Swap ourself in: must be on the swap stack when we do this. We swap the
 * memory directly into the 32-48K area as we know we are live so there is
 * no point whatsoever copying it twice.
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
	swapread(SWAPDEV, blk, 0x4000, 0x8000, p->p_page);
	swapread(SWAPDEV, blk + 0x20, 0x4000, 0xC000, p->p_page2);
#ifdef DEBUG
	kprintf("%x: swapin done %d\n", p, p->p_page);
#endif
}

#endif
