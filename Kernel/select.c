#include <kernel.h>
#include <version.h>
#include <kdata.h>
#include <printf.h>

#ifdef CONFIG_LEVEL_2

/*
 *	Logic for select
 *
 *	Each selectable object needs 3 words of memory (assuming max
 *	16 processes).  If we go over that then this logic is ok but
 *	we would need to trim pipe buffers slightly. Alternatively we
 *	could use separate select objects at that point since presumably
 *	we have enough memory.
 *
 *	While they are 16bits we can instead hide them in the inode copy
 *	in memory for free.
 *
 *	Inode direct block numbers are used as follows
 *	File/Directory: 0-19 all hold disk pointers (not selectable anyway)
 *	Device: 0 holds the device ident, 17-19 hold the select map
 *	Pipe: 0-16 (direct blocks) hold the pipe data block pointers
 *					  17-19 hold the select map
 *	Socket: TODO
 */

uint16_t pipesel = 0;

void seladdwait(struct selmap *s)
{
	uint16_t p = udata.u_ptab - ptab;
	s->map[p >> 3] |= (1 << (p & 7));
}

void selrmwait(struct selmap *s)
{
	uint16_t p = udata.u_ptab - ptab;
	s->map[p >> 3] &= ~(1 << (p & 7));
}

void selwake(struct selmap *s)
{
	ptptr p = ptab;
	uint16_t i;

	for (i = 0; i < maxproc; i++) {
		if (s->map[i >> 3] & (1 << (i & 7)))
			pwake(p);
		p++;
	}
}

/* Set our select bits on the inode */
void selwait_inode(inoptr i, uint_fast8_t smask, uint_fast8_t setit)
{
	struct selmap *s = (struct selmap *) (&i->c_node.i_addr[17]);
	uint_fast8_t bit = udata.u_ptab - ptab;
	uint_fast8_t mask = 1 << (bit & 7);
	uint_fast8_t bset = bit & setit;
	bit >>= 3;

	if (smask & SELECT_IN) {
		s->map[mask] &= ~bit;
		s->map[mask] |= bset;
	}
	s++;
	if (smask & SELECT_OUT) {
		s->map[mask] &= ~bit;
		s->map[mask] |= bset;
	}
	s++;
	if (smask & SELECT_EX) {
		s->map[mask] &= ~bit;
		s->map[mask] |= bset;
	}
}

/* Wake an inode for select */
void selwake_inode(inoptr i, uint16_t mask)
{
	struct selmap *s = (struct selmap *) (&i->c_node.i_addr[17]);
	irqflags_t irq = di();
	if (mask & SELECT_IN)
		selwake(s);
	s++;
	if (mask & SELECT_OUT)
		selwake(s);
	s++;
	if (mask & SELECT_EX)
		selwake(s);
	irqrestore(irq);
}

void selwake_dev(uint_fast8_t major, uint_fast8_t minor, uint16_t mask)
{
	irqflags_t irq = di();
	uint16_t v = (major << 8) | minor;
	inoptr i = i_tab;

	while (i <= &i_tab[ITABSIZE - 1]) {	/* Convoluted form to keep SDCC happy */
		if (i->c_refs && i->c_node.i_addr[0] == v)
			selwake_inode(i, mask);
		i++;
	}
	irqrestore(irq);
}

static int pipesel_begin(inoptr i, uint_fast8_t bits)
{
	uint16_t mask = 0;
	pipesel++;
	/* Data or EOF */
	if (i->c_node.i_size || !i->c_refs)
		mask |= SELECT_IN;
	/* Enough room to be worth waking - keep wakeup rate down */
	if (i->c_node.i_size < 8 * BLKSIZE)
		mask |= SELECT_OUT;
	selwait_inode(i, bits, 1);
	return mask & bits;
}

static void pipesel_end(inoptr i)
{
	pipesel--;
	/* Clear out wait masks */
	selwait_inode(i, SELECT_IN | SELECT_OUT | SELECT_EX, 0);
}

void selwake_pipe(inoptr i, uint16_t mask)
{
	if (pipesel)
		selwake_inode(i, mask);
}


/*******************************************
  _select (nfd, base)            Function 72
  int nfd;
  uint16_t *base;
 *******************************************/
#define nfd (uint16_t)udata.u_argn
#define base (uint16_t *)udata.u_argn1

arg_t _select(void)
{
	irqflags_t irq;
	uint16_t seltype = SELECT_BEGIN;
	inoptr ino;
	uint16_t sumo;
	uint_fast8_t i, m, n;
	uint16_t inr = 0, outr = 0, exr = 0;
	/* Second 16bits of each spare for expansion */
	uint16_t in = ugetw(base);
	uint16_t out = ugetw(base + 2);
	uint16_t ex = ugetw(base + 4);

	uint16_t sum = in | out | ex;

	/* Timeout in 1/10th of a second (BSD api mangling done by libc) */
	/* 0 means return immediately, need to sort out a 'forever' FIXME */
	udata.u_ptab->p_timeout = ugetw(base + 6);
	if (udata.u_ptab->p_timeout)
		ptimer_insert();

	do {
		m = 1;

		irq = di();

		for (i = 0; i < nfd; i++) {
			if (sum & m) {
				if (in & m)
					n = SELECT_IN;
				else
					n = 0;
				if (out & m)
					n |= SELECT_OUT;
				if (ex & m)
					n |= SELECT_EX;
				ino = getinode(i);
				if (ino == NULLINODE)
					return -1;
				switch (getmode(ino)) {
					/* Device types that automatically report some ready states */
				case MODE_R(F_BDEV):
				case MODE_R(F_REG):
					outr |= m;
				case MODE_R(F_DIR):
					inr |= m;
					break;
				case MODE_R(F_PIPE):
					n = pipesel_begin(ino, n);
					goto setbits;
				case MODE_R(F_CDEV):
					/* If unsupported we report the device as read/write ready */
					if (d_ioctl
					    (ino->c_node.i_addr[0],
					     seltype, &n) == -1) {
						udata.u_error = 0;
						n = SELECT_IN | SELECT_OUT;
					} else if (seltype == SELECT_BEGIN)
						selwait_inode(ino, n, 1);
				      setbits:
					/* Set the outputs */
					if (n & SELECT_IN)
						inr |= m;
					if (n & SELECT_OUT)
						outr |= m;
					if (n & SELECT_EX)
						exr |= m;
					break;
				}
			}
			m <<= 1;	/* Next fd mask */
		}
		inr &= in;	/* Don't reply with bits not being selected */
		outr &= out;
		exr &= ex;
		/* FIXME lock against time race */
		sumo = inr | outr | exr;	/* Are we there yet ? */
		/* No successes, wait requested, not timed out */
		if (!sumo && udata.u_ptab->p_timeout > 1 &&
		     psleep_flags(&udata.u_ptab->p_timeout, 0) == -1)
			break;
		irqrestore(irq);
		seltype = SELECT_TEST;
	}
	while (!sumo && udata.u_ptab->p_timeout > 1);

	irqrestore(irq);

	udata.u_ptab->p_timeout = 0;

	/* Return the values to user space */
	uputw(inr, base);
	uputw(outr, base + 2);
	uputw(exr, base + 4);

	/* Tell the device less people care, we may want to remove all this
	   and just select check. The 0 check we could do instead is cheap */
	m = 1;
	for (i = 0; i < nfd; i++) {
		if (sum & m) {
			ino = getinode(i);
			switch (getmode(ino)) {
			case MODE_R(F_CDEV):
				d_ioctl(ino->c_node.i_addr[0], SELECT_END,
					NULL);
				/* Kill the wait */
				selwait_inode(ino, SELECT_IN|SELECT_OUT|SELECT_EX, 0);
				/* May be unknown in which case ignore */
				udata.u_error = 0;
				break;
			case MODE_R(F_PIPE):
				pipesel_end(ino);
			}
		}
		m <<= 1;
	}
	return 0;		/* the scan and return of highest fd is done by
				   user space */
}

#undef nfd
#undef base

#endif
