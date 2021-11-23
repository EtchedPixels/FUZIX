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

static void swapinout(int blk, void* u,
	int (*readwrite)(uint16_t dev, blkno_t blkno, usize_t nbytes,
                    uaddr_t buf, uint16_t page))
{
    panic("cannot swap");
#if 0
	const uint32_t USER_STACK = 3*1024;

	/* Read/write the udata block. */

	readwrite(SWAPDEV, blk, UDATA_BLKS<<BLKSHIFT, (uaddr_t)u, 1);

	/* Data area */

	/* Write out just 0..break and sp..top */
	readwrite(SWAPDEV, blk + UDATA_BLKS,
		(uaddr_t)alignup(udata.u_break - DATABASE, 1<<BLKSHIFT),
		DATABASE, 1);
	readwrite(SWAPDEV, blk + UDATA_BLKS + ((DATALEN - USER_STACK) >> BLKSHIFT),
		USER_STACK, DATABASE + DATALEN - USER_STACK, 1);

	/* Code area */

	readwrite(SWAPDEV, blk + UDATA_BLKS + (DATALEN >> BLKSHIFT),
		(uaddr_t)alignup(udata.u_texttop - CODEBASE, 1<<BLKSHIFT), CODEBASE, 1);
#endif
}

/*
 *	Swap out the memory of a process to make room
 *	for something else
 */
int swapout_new(ptptr p, void *u)
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

	swapinout(blk, u, swapwrite);

	p->p_page = 0;
	p->p_page2 = map;
#ifdef DEBUG
	kprintf("%x: swapout done %d\n", p, p->p_page2);
#endif
	return 0;
}

int swapout(ptptr p)
{
	return swapout_new(p, (uint8_t*)&udata);
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

	swapinout(blk, (uint8_t*)&udata, swapread);

#ifdef DEBUG
	kprintf("%x: swapin done %d\n", p, p->p_page2);
#endif
}

/* vim: sw=4 ts=4 et: */

