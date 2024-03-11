#include <kernel.h>
#include <printf.h>
#include <kdata.h>
#include <stdarg.h>

/* Error checking */

void validchk(uint16_t dev, const char *p)
{
        if (!validdev(dev)) {
                kputs(p);
                kputchar(':');
                panic(PANIC_INVD);
        }
}

/* Buffer pool management */
/*********************************************************************
The high-level interface is through bread() and bfree().
Bread() is given a device and block number, and a rewrite flag.  If
rewrite is 0, the block is actually read if it is not already in the
buffer pool. If rewrite is set, it is assumed that the caller plans to
rewrite the entire contents of the block, and it will not be read in,
but only have a buffer named after it.

Bfree() is given a buffer pointer and a dirty flag.  If the dirty flag
is 0, the buffer is made available for further use.  If the flag is 1,
the buffer is marked "dirty", and it will eventually be written out to
disk.  If the flag is 2, it will be immediately written out.

Zerobuf() returns a buffer of zeroes not belonging to any device.  It
must be bfree'd after use, and must not be dirty. It is used when a
read() wants to read an unallocated block of a file.

Bufsync() write outs all dirty blocks.

Note that a pointer to a buffer structure is the same as a pointer to
the data if the buffer is inline. This is very important.

FIXME: need to add locking to this for the sleeping case, and a hash for
the bigger systems
**********************************************************************/

static uint16_t bufclock;		/* Time-stamp counter for LRU */

#define bisbusy(x)	((x)->bf_busy == BF_BUSY)

#ifndef CONFIG_BLOCK_SLEEP
#define	block(x)	((x)->bf_busy = BF_BUSY)
#define bunlock(x)	((x)->bf_busy = BF_FREE)
#define bcheck(x)	bisbusy(x)
#define block_s(x)
#define bunlock_s(x)
#else

static void block(bufptr bp)
{
	while (bp->bf_busy == BF_BUSY)
		psleep_nosig(bp);
	bp->bf_busy = BF_BUSY;
}

static void bunlock(bufptr bp)
{
	if (bp->bf_busy == BF_FREE)
		panic(BFREEFREE);
	bp->bf_busy = BF_FREE;
	pwake(bp);
}

#define block_s(x)	block(x)
#define bunlock_s(x)	bunlock(x)
#define bcheck(x)	0

#endif

/*
 *	Make an entry in the buffer cache and fill it. If rewrite is
 *	set then we are not keeping any of the old data but overwriting
 *	it all.
 *
 *	Hands back either a locked buffer, or NULL on an error.
 */
bufptr bread(uint16_t dev, blkno_t blk, bool rewrite)
{
	regptr bufptr bp;

	/* TODO speed up the bfind/freebuf into one pass */
	if ((bp = bfind(dev, blk)) == NULL) {
		bp = freebuf();
		bp->bf_dev = dev;
		bp->bf_blk = blk;

		/* If rewrite is set, we are about to write over the entire block,
		   so we don't need the previous contents */
		if (!rewrite) {
			if (bdread(bp) != BLKSIZE) {
				udata.u_error = EIO;
				/* Don't cache the failure */
				bp->bf_dev = NO_DEVICE;
				bp->bf_dirty = false;
				bunlock(bp);
				return (NULL);
			}
		}
	}
	return bp;
}


void brelse(bufptr bp)
{
	bfree(bp, 0);
}

void bawrite(bufptr bp)
{
	bfree(bp, 1);
}

/*
 *	Release an entry in the buffer cache. Passed a locked buffer and
 *	a dirty status
 *
 *	0: Caller did not dirty buffer (but may be dirty already)
 *	1: Caller did dirty buffer
 *	2: Caller dirtied buffer and wants it written back now
 *
 *	If a writeback now is requested an an error occurs then u_error will
 *	be set and -1 returned.
 */
int bfree(regptr bufptr bp, uint_fast8_t dirty)
{				/* dirty: 0=clean, 1=dirty (write back), 2=dirty+immediate write */
	int ret = 0;
	if (dirty)
		bp->bf_dirty = true;
	
	if (dirty > 1) {	/* immediate writeback */
		if (bdwrite(bp) != BLKSIZE) {
			udata.u_error = EIO;
			ret = -1;
		}
		bp->bf_dirty = false;
	}
	/* Time stamp the buffer on free up. It doesn't matter if we stamp it
	   on read or free as while locked it can't go away. However if we
	   stamp it on free we can in future make smarter decisions such as
	   recycling full dirty buffers faster than partial ones */
	bp->bf_time = ++bufclock;
	bunlock(bp);
	return ret;
}

/*
 * Allocate an empty _disk cache_ buffer. We use this when dealing with file
 * holes. It would be nice if this API could go way and readi just use uzero()
 */
bufptr zerobuf(void)
{
	regptr bufptr bp;

	bp = freebuf();
	bp->bf_dev = NO_DEVICE;
	bp->bf_time = ++bufclock;	/* Time stamp it */
	blkzero(bp);
	return bp;
}

#ifdef CONFIG_BLKBUF_HELPERS
/*
 * Allocate a buffer for scratch use by the kernel. This buffer can then
 * be freed with tmpfree.
 *
 * API note: Nothing guarantees a connection between a bufcache entry
 * and tmpbuf in future. Always free with tmpfree.
 */

void *tmpbuf(void)
{
	regptr bufptr bp;

	bp = freebuf();
	bp->bf_dev = NO_DEVICE;
	bp->bf_time = ++bufclock;	/* Time stamp it */
	return bp->__bf_data;
}

void tmpfree(void *p)
{
	brelse(p);
}
#endif

/*
 * Write back a buffer doing the locking outselves. This is called when
 * we do a sync or when we get a media change and need to write back
 * data.
 *
 * FIXME: for the simple case I don't think we can ever get called within
 * an active I/O so the block/bunlock should be fine - but not needed. In
 * async mode they are
 */
static void bdput(regptr bufptr bp)
{
	block_s(bp);
	if (bp->bf_dirty) {
		bdwrite(bp);
		bp->bf_dirty = false;
		bunlock_s(bp);
		d_flush(bp->bf_dev);
	} else
		bunlock_s(bp);
}

/*
 * The low level logic for sync(). We write back each dirty buffer that
 * belongs to a device.
 */
void bufsync(void)
{
	regptr bufptr bp;

	/* FIXME: this can generate a lot of d_flush calls when you have
	   plenty of buffers */
	for (bp = bufpool; bp < bufpool_end; ++bp) {
		if (bp->bf_dev != NO_DEVICE)
		        bdput(bp);
	}
}

/*
 *	Find a matching buffer in the block pool. As we have few buffers
 *	we do a simple linear search. The buffer we return is locked so
 *	that it can't vanish under the caller when we do sleeping block
 *	devices.
 */
bufptr bfind(uint16_t dev, blkno_t blk)
{
	regptr bufptr bp;

	for (bp = bufpool; bp < bufpool_end; ++bp) {
		if (bp->bf_dev == dev && bp->bf_blk == blk) {
			/* FIXME: this check is only relevant for non sync stuff
			   if it's sleeping then this is fine as we'll block here
			   and sleep until the buffer is unlocked */
			if (bcheck(bp))
				panic(PANIC_WANTBSYB);
			block(bp);
			return bp;
		}
	}
	return NULL;
}

/*
 *	Handle umount or media change where we need to discard any old
 *	read buffers.
 *
 *	FIXME: If we want to support mediachange notifications then
 *	we'll need a way to call this that reports errors rather than
 *	trying to write back each block. We'll also need to pass in a mask
 *	for partitioned devices. Maybe the media change case has to be
 *	irq safe ?
 */
void bdrop(uint16_t dev)
{
	regptr bufptr bp;

	for (bp = bufpool; bp < bufpool_end; ++bp) {
		if (bp->bf_dev == dev) {
			bdput(bp);
		        bp->bf_dev = NO_DEVICE;
                }
	}
}

bufptr freebuf(void)
{
	regptr bufptr bp;
	regptr bufptr oldest;
	register int16_t oldtime;

	/* Try to find a non-busy buffer and write out the data if it is dirty */
	oldest = NULL;
	oldtime = 0;
	for (bp = bufpool; bp < bufpool_end; ++bp) {
		if (bufclock - bp->bf_time >= oldtime && !bisbusy(bp)) {
			oldest = bp;
			oldtime = bufclock - bp->bf_time;
		}
	}
	/* FIXME: Once we support sleeping on disk I/O this goes away and
	   we sleep on something - buffer going unbusy or even the oldest
	   buffer and then check if it's still old and if not retry */
	if (!oldest)
		panic(PANIC_NOFREEB);

	block(oldest);
	if (oldest->bf_dirty) {
		bdwrite(oldest);
		oldest->bf_dirty = false;
	}
	return oldest;
}

/*********************************************************************
Bdread() and bdwrite() are the block device interface routines.  They
are given a buffer pointer, which contains the device, block number,
and data location.  They basically validate the device and vector the
call.

Cdread() and cdwrite are the same for character (or "raw") devices,
and are handed a device number.  Udata.u_base, count, and offset have
the rest of the data.
**********************************************************************/

/*********************************************************************
The device driver read and write routines now have only two arguments,
minor and rawflag.  If rawflag is zero, a single block is desired, and
the necessary data can be found in udata.u_buf.  Otherwise, a "raw" or
character read is desired, and udata.u_offset, udata.u_count, and
udata.u_base should be consulted instead.
Any device other than a disk will have only raw access.
**********************************************************************/

static void bdsetup(bufptr bp)
{
	udata.u_buf = bp;
	udata.u_block = bp->bf_blk;
	udata.u_blkoff = 0;
	udata.u_nblock = 1;
	udata.u_dptr = bp->__bf_data;
}

int bdread(bufptr bp)
{
	uint16_t dev = bp->bf_dev;
	validchk(dev, PANIC_BDR);
	bdsetup(bp);
	return ((*dev_tab[major(dev)].dev_read) (minor(dev), 0, 0));
}

int bdwrite(bufptr bp)
{
	uint16_t dev = bp->bf_dev;
	validchk(dev, PANIC_BDW);
	bdsetup(bp);
	return ((*dev_tab[major(dev)].dev_write) (minor(dev), 0, 0));
}

int cdread(uint16_t dev, uint_fast8_t flag)
{
	validchk(dev, PANIC_CDR);
	return ((*dev_tab[major(dev)].dev_read) (minor(dev), 1, flag));
}

int cdwrite(uint16_t dev, uint_fast8_t flag)
{
	validchk(dev, PANIC_CDW);
	return ((*dev_tab[major(dev)].dev_write) (minor(dev), 1, flag));
}

int d_open(uint16_t dev, uint_fast8_t flag)
{
	if (!validdev(dev)) {
	        udata.u_error = ENXIO;
		return -1;
        }
	return ((*dev_tab[major(dev)].dev_open) (minor(dev), flag));
}

int d_close(uint16_t dev)
{
	validchk(dev, PANIC_DCL);
        bdrop(dev);
	return (*dev_tab[major(dev)].dev_close) (minor(dev));
}

int d_ioctl(uint16_t dev, uint16_t request, char *data)
{
	int ret;

	if (!validdev(dev)) {
		udata.u_error = ENXIO;
		return -1;
	}

	ret =  (*dev_tab[major(dev)].dev_ioctl) (minor(dev), request, data);
	/* -1 with no error code means 'unknown ioctl'. Turn this into the
	   right code to save doing it all over */
	if (ret == -1 && !udata.u_error)
			udata.u_error = ENOTTY;
	return ret;
}

int d_flush(uint16_t dev)
{
	/* Not as clean as would be ideal : FIXME */
	int r = d_ioctl(dev, BLKFLSBUF, 0);
	/* Not knowing the ioctl is not an offence */
	if (r && udata.u_error == ENOTTY) {
		udata.u_error = 0;
		r = 0;
	}
	return r;
}

/* 128, 256, 512 supported for now */
static uint16_t masks[3] = { 0x7F, 0xFF, 0x1FF };

/* This is not a commonly used path so can be slower */
int d_blkoff(uint_fast8_t shift)
{
	uint16_t m = masks[shift - 7];
	udata.u_block = udata.u_offset >> shift;
	if (((uint16_t)udata.u_offset) & m) {
		udata.u_error = EIO;
		return -1;
	}
	udata.u_nblock = (udata.u_count + m) >> shift;
	udata.u_dptr = udata.u_base;
	return 0;
}

/*
 *	No such device handler
 */

int nxio_open(uint_fast8_t minor, uint16_t flag)
{
	used(minor);
	used(flag);
	udata.u_error = ENXIO;
	return -1;
}

/*
 *	Default handlers.
 */
int no_open(uint_fast8_t minor, uint16_t flag)
{
	used(minor);
	used(flag);
	return 0;
}

int no_close(uint_fast8_t minor)
{
	used(minor);
	return 0;
}

int no_rdwr(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag)
{
	used(minor);
	used(rawflag);
	used(flag);
	udata.u_error = EINVAL;
	return -1;
}

int no_ioctl(uint_fast8_t minor, uarg_t a, char *b)
{
	used(minor);
	used(a);
	used(b);
	udata.u_error = ENOTTY;
	return -1;
}

/*
 *             Character queue management routines
 */

/* add something to the tail of the queue. */
bool insq(struct s_queue * qp, uint_fast8_t c)
{
	regptr struct s_queue *q = qp;
	bool r;

	irqflags_t irq = di();

	if (q->q_count == q->q_size)
		r = false;	// no space left :(
	else {
		PUTQ(q->q_tail, c);
		++q->q_count;
		if (++q->q_tail >= q->q_base + q->q_size)
			q->q_tail = q->q_base;
		r = true;
	}
	irqrestore(irq);
	return r;
}


/* Remove something from the head of the queue. */
bool remq(struct s_queue * qp, uint_fast8_t *cp)
{
	regptr struct s_queue *q = qp;
	bool r;

	irqflags_t irq = di();

	if (!q->q_count)
		r = false;
	else {
		*cp = GETQ(q->q_head);
		--q->q_count;
		if (++q->q_head >= q->q_base + q->q_size)
			q->q_head = q->q_base;
		r = true;
	}

	irqrestore(irq);
	return r;
}


/* Clear the queue to empty conditions.  (UZI280 addition) */
void clrq(struct s_queue *qp)
{
	regptr struct s_queue *q = qp;
	irqflags_t irq = di();

	q->q_head = q->q_tail = q->q_base;
	q->q_count = 0;

	irqrestore(irq);
}


/* Remove something from the tail; the most recently added char. */
bool uninsq(struct s_queue *qp, uint_fast8_t *cp)
{
	regptr struct s_queue *q = qp;
	bool r;
	irqflags_t irq = di();

	if (!q->q_count)
		r = false;
	else {
		--q->q_count;
		if (--q->q_tail < q->q_base)
			q->q_tail = q->q_base + q->q_size - 1;
		*cp = GETQ(q->q_tail);
		r = true;
	}
	irqrestore(irq);
	return r;
}

/* Returns true if the queue has more characters than its wakeup number */
bool fullq(struct s_queue *q)
{
    if (q->q_count > q->q_wakeup) // WRS: shouldn't this be >= ?
        return true;
    else
        return false;
}

/*********************************************************************
             Miscellaneous helpers
**********************************************************************/

int psleep_flags_io(void *p, uint_fast8_t flags)
{
	if (flags & O_NDELAY) {
	        if (!udata.u_done) {
	                udata.u_done = (usize_t)-1;
			udata.u_error = EAGAIN;
                }
		return -1;
	}
	psleep(p);
	if (chksigs()) {
	        if (!udata.u_done) {
	                udata.u_done = (usize_t)-1;
                        udata.u_error = EINTR;
                }
		return -1;
	}
	return 0;
}

int psleep_flags(void *p, uint_fast8_t flags)
{
	if (flags & O_NDELAY) {
		udata.u_error = EAGAIN;
		return -1;
	}
	psleep(p);
	if (chksigs()) {
                udata.u_error = EINTR;
		return -1;
	}
	return 0;
}

void kputs(const char *p)
{
	while (*p)
		kputchar(*p++);
}

static void putdigit0(uint_fast8_t c)
{
	kputchar("0123456789ABCDEF"[c & 15]);
}

static void putdigit(uint_fast8_t c, unsigned char *flag)
{
	if (c || *flag) {
		*flag |= c;
		putdigit0(c);
	}
}

void kputhex(unsigned int v)
{
	putdigit0(v >> 12);
	putdigit0(v >> 8);
	putdigit0(v >> 4);
	putdigit0(v);
}

void kputhexbyte(unsigned int v)
{
	putdigit0(v >> 4);
	putdigit0(v);
}

void kputunum(unsigned int v)
{
	unsigned char n = 0;
	putdigit((v / 10000) % 10, &n);
	putdigit((v / 1000) % 10, &n);
	putdigit((v / 100) % 10, &n);
	putdigit((v / 10) % 10, &n);
	putdigit0(v % 10);
}

void kputnum(int v)
{
	if (v < 0) {
		kputchar('-');
		v = -v;
	}
	kputunum(v);
}

void kprintf(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	while (*fmt) {
		if (*fmt == '%') {
			fmt++;
			switch (*fmt) {
				case 's':
				{
					char* str = va_arg(ap, char *);
					kputs(str);
					fmt++;
					continue;
				}

				case 'c':
				{
					char c = va_arg(ap, int);
					kputchar(c);
					fmt++;
					continue;
				}
#ifdef CONFIG_32BIT
				case 'p':
					fmt--;
#endif
				case 'l': /* assume an x is following */
				{
					long l = va_arg(ap, unsigned long);
					/* TODO: not 32-bit safe */
					kputhex((uint16_t)(l >> 16));
					kputhex((uint16_t)l);
					fmt += 2;
					continue;
				}

				case '2': /* assume an x is following */
				{
					char c = va_arg(ap, int);
					kputhexbyte(c);
					fmt += 2;
					continue;
				}

#ifndef CONFIG_32BIT
				case 'p':
#endif
				case 'x':
				case 'd':
				case 'u':
				{
					unsigned int v = va_arg(ap, int);

					if (*fmt == 'x' || *fmt == 'p')
						kputhex(v);
					else if (*fmt == 'd')
						kputnum(v);
					else if (*fmt == 'u')
						kputunum(v);

					fmt++;
					continue;
				}
			}
		}
		kputchar(*fmt);
		fmt++;
	}

	va_end(ap);
}

#ifdef CONFIG_IDUMP

void bufdump(void)
{
	bufptr j;

	kprintf("\ndev\tblock\tdirty\tbusy\ttime clock %d\n", bufclock);
	for (j = bufpool; j < bufpool_end; ++j)
		kprintf("%d\t%u\t%d\t%d\t%u\n", j->bf_dev, j->bf_blk,
			j->bf_dirty, j->bf_busy, j->bf_time);
}

void idump(void)
{
	inoptr ip;
	ptptr pp;
	extern struct cinode i_tab[];

	kprintf("Err %d root %d\n", udata.u_error, root - i_tab);
	kputs("\tMAGIC\tDEV\tNUM\tMODE\tNLINK\t(DEV)\tREFS\tR/W\tDIRTY\n");

	for (ip = i_tab; ip < i_tab + ITABSIZE; ++ip) {
		if(ip->c_magic != CMAGIC)
			continue;
		kprintf("%d\t%d\t%d\t%u\t%u\t",
			ip - i_tab, ip->c_magic, ip->c_dev, ip->c_num,
			ip->c_node.i_mode);
		kprintf("%d\t%d\t%d\t%d:%d\t%d\n",
			ip->c_node.i_nlink, ip->c_node.i_addr[0],
			ip->c_refs, ip->c_readers,ip->c_writers, ip->c_flags);
	}

	kputs
	    ("\n\tSTAT\tWAIT\tPID\tPPTR\tALARM\tPENDING\t\tIGNORED\n");
	for (pp = ptab; pp < ptab + PTABSIZE /*maxproc */ ; ++pp) {
		if (pp->p_status == P_EMPTY)
			continue;
		kprintf("%d\t%d\t0x%x\t%d\t",
			pp - ptab, pp->p_status, pp->p_wait, pp->p_pid);
		kprintf("%d\t%d\t0x%x%x\t0x%x%x\n",
			pp->p_pptr - ptab, pp->p_alarm, 
			/* kprintf has no %lx so we write out 32-bit
			 * values as two 16-bit values instead */
			pp->p_sig[0].s_pending, pp->p_sig[1].s_pending,
			pp->p_sig[0].s_ignored, pp->p_sig[1].s_ignored);
	}

	bufdump();

	kprintf("insys %d ptab %d call %d cwd %d sp 0x%x\n",
		udata.u_insys, udata.u_ptab - ptab, udata.u_callno,
		udata.u_cwd - i_tab, udata.u_sp);
}

#endif
