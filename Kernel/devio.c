#include <kernel.h>
#include <printf.h>
#include <kdata.h>
#include <stdarg.h>

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
the data.  This is very important.
**********************************************************************/

uint16_t bufclock;		/* Time-stamp counter for LRU */

uint8_t *bread(uint16_t dev, blkno_t blk, bool rewrite)
{
	register bufptr bp;

	if ((bp = bfind(dev, blk)) != NULL) {
		if (bp->bf_busy)
			panic("want busy block");
		goto done;
	}
	bp = freebuf();
	bp->bf_dev = dev;
	bp->bf_blk = blk;

	/* If rewrite is set, we are about to write over the entire block,
	   so we don't need the previous contents */
	if (!rewrite) {
		if (bdread(bp) == -1) {
			udata.u_error = EIO;
			return (NULL);
		}
	}

      done:
	bp->bf_busy = 1;
	bp->bf_time = ++bufclock;	/* Time stamp it */
	return (bp->bf_data);
}


void brelse(void *bp)
{
	bfree((bufptr) bp, 0);
}


void bawrite(void *bp)
{
	bfree((bufptr) bp, 1);
}


int bfree(bufptr bp, uint8_t dirty)
{				/* dirty: 0=clean, 1=dirty (write back), 2=dirty+immediate write */
	if (dirty)
		bp->bf_dirty = true;
	bp->bf_busy = false;

	if (dirty > 1) {	// immediate writeback
		if (bdwrite(bp) == -1)
			udata.u_error = EIO;
		bp->bf_dirty = false;
		return -1;
	}
	return 0;
}


/* This returns a busy block not belonging to any device, with
 * garbage contents.  It is essentially a malloc for the kernel.
 * Free it with brelse()!
 */
void *tmpbuf(void)
{
	bufptr bp;

	bp = freebuf();
	bp->bf_dev = NO_DEVICE;
	bp->bf_busy = true;
	bp->bf_time = ++bufclock;	/* Time stamp it */
	return bp->bf_data;
}


void *zerobuf(void)
{
	void *b;

	b = tmpbuf();
	memset(b, 0, 512);

	return b;
}


void bufsync(void)
{
	bufptr bp;

	for (bp = bufpool; bp < bufpool + NBUFS; ++bp) {
		if ((bp->bf_dev != NO_DEVICE) && bp->bf_dirty) {
			bdwrite(bp);
			if (!bp->bf_busy)
				bp->bf_dirty = false;
		}
	}
}

bufptr bfind(uint16_t dev, blkno_t blk)
{
	bufptr bp;

	for (bp = bufpool; bp < bufpool + NBUFS; ++bp) {
		if (bp->bf_dev == dev && bp->bf_blk == blk)
			return bp;
	}
	return NULL;
}


bufptr freebuf(void)
{
	bufptr bp;
	bufptr oldest;
	int16_t oldtime;

	/* Try to find a non-busy buffer and write out the data if it is dirty */
	oldest = NULL;
	oldtime = 0;
	for (bp = bufpool; bp < bufpool + NBUFS; ++bp) {
		if (bufclock - bp->bf_time >= oldtime && !bp->bf_busy) {
			oldest = bp;
			oldtime = bufclock - bp->bf_time;
		}
	}
	if (!oldest)
		panic("no free buffers");

	if (oldest->bf_dirty) {
		if (bdwrite(oldest) == -1)
			udata.u_error = EIO;
		oldest->bf_dirty = false;
	}
	return oldest;
}


/*
 *	Helper for hinting that a buffer is not likely to be re-read rapidly
 *	Ignores the hint if the buffer is dirty, resets it if the buffer is
 *	requested again
 */
void bufdiscard(bufptr bp)
{
	if (!bp->bf_dirty)
		/* Make this the oldest buffer */
		bp->bf_time = bufclock - 1000;
}

void bufdump(void)
{
	bufptr j;

	kprintf("\ndev\tblock\tdirty\tbusy\ttime clock %d\n", bufclock);
	for (j = bufpool; j < bufpool + NBUFS; ++j)
		kprintf("%d\t%u\t%d\t%d\t%u\n", j->bf_dev, j->bf_blk,
			j->bf_dirty, j->bf_busy, j->bf_time);
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

/* FIXME: To do banking without true 'far' pointers we need to figure
   out some scheme to do a bank call here - do we need a dev_tab bank
   entry perhaps ? */

int bdread(bufptr bp)
{
	uint16_t dev = bp->bf_dev;
	if (!validdev(dev))
		panic("bdread: invalid dev");

	udata.u_buf = bp;
	return ((*dev_tab[major(dev)].dev_read) (minor(dev), 0, 0));
}


int bdwrite(bufptr bp)
{
	uint16_t dev = bp->bf_dev;
	if (!validdev(dev))
		panic("bdwrite: invalid dev");

	udata.u_buf = bp;
	return ((*dev_tab[major(dev)].dev_write) (minor(dev), 0, 0));
}

int cdread(uint16_t dev, uint8_t flag)
{
	if (!validdev(dev))
		panic("cdread: invalid dev");
	return ((*dev_tab[major(dev)].dev_read) (minor(dev), 1, flag));
}

int cdwrite(uint16_t dev, uint8_t flag)
{
	if (!validdev(dev))
		panic("cdwrite: invalid dev");
	return ((*dev_tab[major(dev)].dev_write) (minor(dev), 1, flag));
}

// WRS: swapread(), swapwrite() removed.

int d_open(uint16_t dev, uint8_t flag)
{
	if (!validdev(dev))
		return -1;
	return ((*dev_tab[major(dev)].dev_open) (minor(dev), flag));
}


/* FIXME: on the last close we ought to flush/invalidate any bufs
   for this device so we can support swapping between media properly
   (right now the cache is so small it happens to work...) */
int d_close(uint16_t dev)
{
	if (!validdev(dev))
		panic("d_close: bad device");
	return (*dev_tab[major(dev)].dev_close) (minor(dev));
}

int d_ioctl(uint16_t dev, uint16_t request, char *data)
{
	if (!validdev(dev)) {
		udata.u_error = ENXIO;
		return -1;
	}

	if ((*dev_tab[major(dev)].dev_ioctl) (minor(dev), request, data)) {
		if (!udata.u_error)	// maybe the ioctl routine might set this?
			udata.u_error = EINVAL;
		return -1;
	}

	return 0;
}

/*
 *	No such device handler
 */

int nxio_open(uint8_t minor, uint16_t flag)
{
	minor;
	flag;
	udata.u_error = ENXIO;
	return -1;
}

/*
 *	Default handlers.
 */
int no_open(uint8_t minor, uint16_t flag)
{
	minor;
	flag;
	return 0;
}

int no_close(uint8_t minor)
{
	minor;
	return 0;
}

int no_rdwr(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
	minor;
	rawflag;
	flag;
	udata.u_error = EINVAL;
	return -1;
}

int no_ioctl(uint8_t minor, uint16_t a, char *b)
{
	minor;
	a;
	b;
	udata.u_error = ENOTTY;
	return -1;
}

/*
 *             Character queue management routines
 */

/* add something to the tail of the queue. */
bool insq(struct s_queue * q, unsigned char c)
{
	bool r;

	irqflags_t irq = di();

	if (q->q_count == q->q_size)
		r = false;	// no space left :(
	else {
		*(q->q_tail) = c;
		++q->q_count;
		if (++q->q_tail >= q->q_base + q->q_size)
			q->q_tail = q->q_base;
		r = true;
	}
	irqrestore(irq);
	return r;
}


/* Remove something from the head of the queue. */
bool remq(struct s_queue * q, unsigned char *cp)
{
	bool r;

	irqflags_t irq = di();

	if (!q->q_count)
		r = false;
	else {
		*cp = *(q->q_head);
		--q->q_count;
		if (++q->q_head >= q->q_base + q->q_size)
			q->q_head = q->q_base;
		r = true;
	}

	irqrestore(irq);
	return r;
}


/* Clear the queue to empty conditions.  (UZI280 addition) */
void clrq(struct s_queue *q)
{
	irqflags_t irq = di();

	q->q_head = q->q_tail = q->q_base;
	q->q_count = 0;

	irqrestore(irq);
}


/* Remove something from the tail; the most recently added char. */
bool uninsq(struct s_queue *q, unsigned char *cp)
{
	bool r;
	irqflags_t irq = di();

	if (!q->q_count)
		r = false;
	else {
		--q->q_count;
		if (--q->q_tail < q->q_base)
			q->q_tail = q->q_base + q->q_size - 1;
		*cp = *(q->q_tail);
		r = true;
	}
	irqrestore(irq);
	return r;
}

// WRS: this isn't used
// /* Returns true if the queue has more characters than its wakeup number
//  */
// bool fullq(struct s_queue *q)
// {
//     if (q->q_count > q->q_wakeup) // WRS: shouldn't this be >= ?
//         return true;
//     else
//         return false;
// }

/*********************************************************************
             Miscellaneous helpers
**********************************************************************/

int psleep_flags(void *p, unsigned char flags)
{
	if (flags & O_NDELAY) {
		udata.u_error = EAGAIN;
		return (-1);
	}
	psleep(p);
	if (udata.u_cursig || udata.u_ptab->p_pending) {	/* messy */
		udata.u_error = EINTR;
		return (-1);
	}
	return 0;
}

void kputs(const char *p)
{
	while (*p)
		kputchar(*p++);
}

static void putdigit0(unsigned char c)
{
	kputchar("0123456789ABCDEF"[c & 15]);
}

static void putdigit(unsigned char c, unsigned char *flag)
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
	char *str;
	unsigned int v;
	char c;
	va_list ap;

	va_start(ap, fmt);
	while (*fmt) {
		if (*fmt == '%') {
			fmt++;
			if (*fmt == 's') {
				str = va_arg(ap, char *);
				kputs(str);
				fmt++;
				continue;
			}
			if (*fmt == 'c') {
				c = va_arg(ap, int);
				kputchar(c);
				fmt++;
				continue;
			}
			if (*fmt == 'x' || *fmt == 'd' || *fmt == 'u') {
				v = va_arg(ap, int);
				if (*fmt == 'x')
					kputhex(v);
				else if (*fmt == 'd')
					kputnum(v);
				else if (*fmt == 'u')
					kputunum(v);
				fmt++;
				continue;
			}
		}
		kputchar(*fmt);
		fmt++;
	}
}

#ifdef CONFIG_IDUMP

void idump(void)
{
	inoptr ip;
	ptptr pp;
	extern struct cinode i_tab[];

	kprintf("Err %d root %d\n", udata.u_error, root - i_tab);
	kputs("\tMAGIC\tDEV\tNUM\tMODE\tNLINK\t(DEV)\tREFS\tDIRTY\n");

	for (ip = i_tab; ip < i_tab + ITABSIZE; ++ip) {
		kprintf("%d\t%d\t%d\t%u\t0%o\t",
			ip - i_tab, ip->c_magic, ip->c_dev, ip->c_num,
			ip->c_node.i_mode);
		kprintf("%d\t%d\t%d\t%d\n",	/* line split for compiler */
			ip->c_node.i_nlink, ip->c_node.i_addr[0],
			ip->c_refs, ip->c_flags);
		if (!ip->c_magic)
			break;
	}

	kputs
	    ("\n\tSTAT\tWAIT\tPID\tPPTR\tALARM\tPENDING\tIGNORED\tCHILD\n");
	for (pp = ptab; pp < ptab + PTABSIZE /*maxproc */ ; ++pp) {
		if (pp->p_status == P_EMPTY)
			continue;
		kprintf("%d\t%d\t0x%x\t%d\t",
			pp - ptab, pp->p_status, pp->p_wait, pp->p_pid);
		kprintf("%d\t%d\t0x%lx\t0x%lx\n",
			pp->p_pptr - ptab, pp->p_alarm, pp->p_pending,
			pp->p_ignored);
	}

	bufdump();

	kprintf("insys %d ptab %d call %d cwd %d sp 0x%x\n",
		udata.u_insys, udata.u_ptab - ptab, udata.u_callno,
		udata.u_cwd - i_tab, udata.u_sp);
}

#endif
