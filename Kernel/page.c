#include <kernel.h>

#ifdef PAGEDEV

#include <kdata.h>
#include <page.h>

/*
 *	General purpose helpers used by paging based memory mappers as
 *	opposed to swap based ones.
 *
 *	We make the underlying disk I/O look the same for paging and
 *	swapping.
 */

uint16_t swappage;

/* We can re-use udata.u_block and friends as we can never be swapped while
   we are in the middle of an I/O (at least for now). If we rework the kernel
   for sleepable I/O this will change */
int pageread(uint16_t dev, blkno_t blkno, usize_t nbytes,
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
	kprintf("PR | L %p M %p Bytes %u Page %u Block %u\n",
		buf, udata.u_dptr, nbytes, page, udata.u_block);
#endif
	res = ((*dev_tab[major(dev)].dev_read) (minor(dev), 2, 0));
		
#ifdef CONFIG_TRIM
	while (nbytes != 0)
	{
		d_ioctl(dev, HDIO_TRIM, (void*)&blkno);
		blkno++;
		nbytes -= 1 << BLKSHIFT;
	}
#endif

	return res;
}

int pagewrite(uint16_t dev, blkno_t blkno, usize_t nbytes,
		     uaddr_t buf, uint16_t page)
{
	udata.u_dptr = swap_map(buf);
	udata.u_block = blkno;
	if (nbytes & BLKMASK)
		panic("swpwr");
	udata.u_nblock = nbytes >> BLKSHIFT;
	swappage = page;

#ifdef DEBUG
	kprintf("PW | L %p M %p Bytes %u Page %u Block %u\n",
		buf, udata.u_dptr, nbytes, page, udata.u_block);
#endif
	return ((*dev_tab[major(dev)].dev_write) (minor(dev), 2, 0));
}

#endif
