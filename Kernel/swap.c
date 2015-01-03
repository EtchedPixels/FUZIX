/*
 *	Very simplistic for now disk swap logic. As we have a fixed
 *	process size at this point we use a fixed swap size too
 */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>

#undef DEBUG

#ifdef SWAPDEV

#ifndef UDATA_SWAPSIZE
#define UDATA_BLOCKS	0
#endif

#ifndef CONFIG_COMMON_COPY
#define flush_cache(p)	do {} while(0)
#endif

uint8_t *swapbase;
unsigned int swapcnt;
blkno_t swapblk;
ptptr swapproc;			/* Target process space */

/* Table of available maps */
static uint8_t swapmap[MAX_SWAPS];
static uint8_t swapptr = 0;

void swapmap_add(uint8_t swap)
{
	if (swapptr == MAX_SWAPS)
		panic("maxswap");
	swapmap[swapptr++] = swap;
}

static int swapread(uint16_t dev, blkno_t blkno, unsigned int nbytes,
                    uint8_t *buf)
{
	swapbase = buf;
	swapcnt = nbytes;
	swapblk = blkno;
	return ((*dev_tab[major(dev)].dev_read) (minor(dev), 2, 0));
}


static int swapwrite(uint16_t dev, blkno_t blkno, unsigned int nbytes,
		     uint8_t *buf)
{
	swapbase = buf;
	swapcnt = nbytes;
	swapblk = blkno;
	return ((*dev_tab[major(dev)].dev_write) (minor(dev), 2, 0));
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
#ifdef UDATA_SWAPSIZE
	uint8_t *ptr;
#endif

	swapproc = p;

	if (page) {
#ifdef DEBUG
		kprintf("Swapping out %x (%d)\n", p, p->p_page);
#endif
		/* Are we out of swap ? */
		if (swapptr == 0)
			return ENOMEM;
                flush_cache(p);
		map = swapmap[--swapptr];
		blk = map * SWAP_SIZE;
#ifdef UDATA_SWAPSIZE
		/* Allow the platform to do things like handle the uarea stash */
		ptr = swapout_prepare_uarea(p);
		/* Write to disk if the platform asks us */
		if (ptr)
			swapwrite(SWAPDEV, blk, UDATA_SWAPSIZE,
				  ptr);
#endif
		/* Write the app (and possibly the uarea etc..) to disk */
		swapwrite(SWAPDEV, blk + UDATA_BLOCKS, SWAPTOP - SWAPBASE,
			  SWAPBASE);
		pagemap_free(p);
		p->p_page = 0;
		p->p_page2 = map;
#ifdef DEBUG
		kprintf("%x: swapout done %d\n", p, p->p_page);
#endif
	}
#ifdef DEBUG
	else
		kprintf("%x: process already swapped!\n", p);
#endif
	return 0;
}

/*
 * Swap ourself in: must be on the swap stack when we do this
 */
static void swapin(ptptr p)
{
	uint16_t blk = p->p_page2 * SWAP_SIZE;
#ifdef UDATA_SWAPSIZE
	uint8_t *ptr;
#endif

#ifdef DEBUG
	kprintf("Swapin %x, %d\n", p, p->p_page);
#endif
	if (!p->p_page) {
		kprintf("%x: nopage!\n", p);
		return;
	}

	/* Return our swap */
	swapmap[swapptr++] = p->p_page2;

	swapproc = p;		/* always ourself */
	swapread(SWAPDEV, blk + UDATA_BLOCKS, SWAPTOP - SWAPBASE,
		 SWAPBASE);
#ifdef UDATA_SWAPSIZE
	ptr = swapin_prepare_uarea(p);
	if (ptr)
		swapread(SWAPDEV, blk, UDATA_SWAPSIZE, (uint8_t *) &udata);
#endif
#ifdef DEBUG
	kprintf("%x: swapin done %d\n", p, p->p_page);
#endif
}

/*
 *	Swap out process. As we have them all the same size we ignore
 *	p for now. For a single memory image system most of this can go!
 */
static ptptr swapvictim(ptptr p, int notself)
{
#ifdef CONFIG_MULTI
	ptptr c;
	ptptr r = NULL;
	ptptr f = NULL;
	uint16_t sc = 0;
	uint16_t s;

	extern ptptr getproc_nextp;	/* Eww.. */

	c = getproc_nextp;

	do {
		if (c->p_page) {	/* No point swapping someone in swap! */
			/* Find the last entry before us */
			if (c->p_status == P_READY)
				r = c;
			if (c->p_status > P_READY
			    && p->p_status <= P_FORKING) {
				/* relative position in order of waits, bigger is longer, can wrap but
				   shouldn't really matter to us much if it does */
				s = (waitno - c->p_waitno);
				if (s >= sc) {
					sc = s;
					f = c;
				}
			}
		}
		c++;
		if (c == ptab_end)
			c = ptab;
	}
	while (c != getproc_nextp);

	/* Oldest waiter gets the boot */
	if (f) {
#ifdef DEBUG
		kprintf("swapvictim %x (page %d, state %d\n)", f,
			f->p_page, f->p_status);
#endif
		return f;
	}
	/* No waiters.. the scheduler cycles so we will be the last to run
	   again, failing that the one before us that was ready */
	if (notself == 0)
		return udata.u_ptab;
	return r;
#else
	p;
	if (notself)
		panic("bad notself\n");
	return udata.u_ptab;
#endif
}

ptptr swapneeded(ptptr p, int notself)
{
	ptptr n = swapvictim(p, notself);
	if (n)
		if (swapout(n))
			n = NULL;
	return n;
}

/*
 *	Called from switchin when we discover that we want to run
 *	a swapped process. As all our processes are the same size
 *	for now this remains fairly simple.
 */
void swapper(ptptr p)
{
	pagemap_alloc(p);	/* May cause a swapout */
#ifdef DEBUG
	kprintf("Swapping in %x (page %d), utab.ptab %x\n", p, p->p_page,
		udata.u_ptab);
#endif
	swapin(p);
#ifdef DEBUG
	kprintf("Swapped in %x (page %d), udata.ptab %x\n",
		p, p->p_page, udata.u_ptab);
#endif
}
#endif
