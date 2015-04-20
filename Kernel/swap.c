/*
 *	Very simplistic for now disk swap logic. As we have a fixed
 *	process size at this point we use a fixed swap size too
 */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>

#undef DEBUG

#ifdef SWAPDEV


uint8_t *swapbase;
unsigned int swapcnt;
blkno_t swapblk;
uint16_t swappage;			/* Target page */

/* Table of available maps */
static uint8_t swapmap[MAX_SWAPS];
static uint8_t swapptr = 0;

void swapmap_add(uint8_t swap)
{
	if (swapptr == MAX_SWAPS)
		panic("maxswap");
	swapmap[swapptr++] = swap;
}

int swapmap_alloc(void)
{
        if (swapptr)
                return swapmap[--swapptr];
        else
                return 0;
}

/* FIXME: clean this up by having a common i/o structure to avoid
   all the mode 1 and mode 2 confusion and conversions */

int swapread(uint16_t dev, blkno_t blkno, unsigned int nbytes,
                    uint16_t buf, uint16_t page)
{
	swapbase = swap_map(buf);
	swapcnt = nbytes;
	swapblk = blkno;
	swappage = page;
	return ((*dev_tab[major(dev)].dev_read) (minor(dev), 2, 0));
}


int swapwrite(uint16_t dev, blkno_t blkno, unsigned int nbytes,
		     uint16_t buf, uint16_t page)
{
	swapbase = swap_map(buf);
	swapcnt = nbytes;
	swapblk = blkno;
	swappage = page;
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
	used(p);
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
 *	a swapped process. We let pagemap_alloc cause any needed swap
 *	out of idle processes.
 */
void swapper(ptptr p)
{
        uint16_t map = p->p_page2;
	pagemap_alloc(p);	/* May cause a swapout. May also destroy
                                   the old value of p->page2 */
#ifdef DEBUG
	kprintf("Swapping in %x (page %d), utab.ptab %x\n", p, p->p_page,
		udata.u_ptab);
#endif
	swapin(p, map);
	swapmap_add(map);
#ifdef DEBUG
	kprintf("Swapped in %x (page %d), udata.ptab %x\n",
		p, p->p_page, udata.u_ptab);
#endif
}
#endif
