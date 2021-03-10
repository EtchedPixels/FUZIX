/*
 *	For small machines it's cheaper to simply allocate banks in the size
 *	needed for the largest swapout of the application as that'll be under
 *	64K. For split I/D we can allocate pairs of swap banks.
 *
 *	It's possible to be a lot smarter about this and for 32bit systems
 *	it becomes a necessity not to use this simple swap logic.
 */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>

#undef DEBUG

#ifdef SWAPDEV


uint16_t swappage;			/* Target page */

/* Table of available maps */
static uint8_t swapmap[MAX_SWAPS];
static uint_fast8_t swapptr = 0;

static char maxswap[] = PANIC_MAXSWAP;

void swapmap_add(uint_fast8_t swap)
{
	if (swapptr == MAX_SWAPS)
		panic(maxswap);
	swapmap[swapptr++] = swap;
	sysinfo.swapusedk -= SWAP_SIZE / 2;
}

int swapmap_alloc(void)
{
        if (swapptr) {
		sysinfo.swapusedk += SWAP_SIZE / 2;
                return swapmap[--swapptr];
	}
        else
                return -1;
}

void swapmap_init(uint_fast8_t swap)
{
	if (swapptr == MAX_SWAPS)
		panic(maxswap);
	swapmap[swapptr++] = swap;
	sysinfo.swapk += SWAP_SIZE / 2;
}

/* We can re-use udata.u_block and friends as we can never be swapped while
   we are in the middle of an I/O (at least for now). If we rework the kernel
   for sleepable I/O this will change */

int swapread(uint16_t dev, blkno_t blkno, usize_t nbytes,
                    uaddr_t buf, uint16_t page)
{
	int res;

	udata.u_dptr = swap_map(buf);
	udata.u_block = blkno;
	if (nbytes & BLKMASK)
		panic("swprd");
	udata.u_nblock = nbytes >> BLKSHIFT;
	swappage = page;
#ifdef DEBUG
	kprintf("SR | L %p M %p Bytes %d Page %d Block %d\n",
		buf, udata.u_dptr, nbytes, page, udata.u_block);
#endif
	res = ((*dev_tab[major(dev)].dev_read) (minor(dev), 2, 0));

#ifdef CONFIG_TRIM
	while (nbytes != 0)
	{
		d_ioctl(dev, HDIO_TRIM, (void*)&blkno);
		blkno++;
		nbytes -= 1<<BLKSHIFT;
	}
#endif

	return res;
}


int swapwrite(uint16_t dev, blkno_t blkno, usize_t nbytes,
		     uaddr_t buf, uint16_t page)
{
	/* FIXME: duplication here */
	udata.u_dptr = swap_map(buf);
	udata.u_block = blkno;
	if (nbytes & BLKMASK)
		panic("swpwr");
	udata.u_nblock = nbytes >> BLKSHIFT;
	swappage = page;
#ifdef DEBUG
	kprintf("SW | L %p M %p Bytes %d Page %d Block %d\n",
		buf, udata.u_dptr, nbytes, page, udata.u_block);
#endif
	return ((*dev_tab[major(dev)].dev_write) (minor(dev), 2, 0));
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

	/* FIXME: with the punishment flag we ought to also consider that */
	do {
		if (c->p_page && c != udata.u_ptab) {	/* No point swapping someone in swap! */
			/* Find the last entry before us */
			if (c->p_status == P_READY)
				r = c;
			if (c->p_status > P_READY
			    && c->p_status <= P_FORKING) {
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
		kprintf("swapvictim %x (page %d, state %d)\n", f,
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
	used(p);
	if (notself)
		panic(PANIC_NOTSELF);
	return udata.u_ptab;
#endif
}

ptptr swapneeded(ptptr p, int notself)
{
	ptptr n = swapvictim(p, notself);
	if (n) {
		inswap = 1;
		if (swapout(n))
			n = NULL;
		inswap = 0;
	}
	return n;
}

void swapper2(ptptr p, uint16_t map)
{
#ifdef DEBUG
	kprintf("Swapping in %p (page %d), utab.ptab %p\n", p, p->p_page,
		udata.u_ptab);
#endif
	swapin(p, map);
	swapmap_add(map);
#ifdef DEBUG
	kprintf("Swapped in %p (page %d), udata.ptab %p\n",
		p, p->p_page, udata.u_ptab);
#endif
	udata.u_page = p->p_page;
	udata.u_page2 = p->p_page2;
	/* We booted it out onto disk, don't then not run it */
	p->p_flags &= ~PFL_BATCH;
}

/*
 *	Called from switchin when we discover that we want to run
 *	a swapped process. We let pagemap_alloc cause any needed swap
 *	out of idle processes.
 */
void swapper(ptptr p)
{
        uint16_t map = p->p_page2;
	pagemap_alloc(p);	/* May cause a swapout. May also destroy
                                   the old value of p->page2 */
	swapper2(p, map);
}

#endif
