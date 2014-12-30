#include <kernel.h>
#include <version.h>
#include <kdata.h>
#include <printf.h>

/*
 *	Logic for select
 *
 *	Each selectable object needs 3 words of memory (assuming max
 *	16 processes).  If we go over that then this logic is ok but
 *	we would need to trim pipe buffers slightly.
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

void seladdwait(struct selmap *s)
{
  uint16_t p = udata.u_ptab - ptab;
  s->map[p>>3] |= (1 << (p & 7));
}

void selrmwait(struct selmap *s)
{
  uint16_t p = udata.u_ptab - ptab;
  s->map[p>>3] &= ~(1 << (p & 7));
}

void selwake(struct selmap *s)
{
  p = ptab;
  for (i = 0; i < maxproc; i++) {
    if (s->map[i>>3] & (1 << (i & 7)))
      pwake(p);
    p++;
  }
}

/* Set our select bits on the inode */
int selwait_inode(inoptr i, uint8_t smask, uint8_t setit) {
  struct selmap *s = (struct selmap *)(&i->c_node.i_addr[17]);
  uint8_t bit = udata.u_ptab - ptab;
  uint8_t mask = 1 << (bit & 7);
  uint8_t bset = bit & setit;
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
void selwake_inode(inoptr i, uint16_t mask) {
  struct selmap *s = (struct selmap *)(&i->c_node.i_addr[17]);
  if (mask & SELECT_IN)
    selwake(s);
  s++;
  if (mask & SELECT_OUT)
    selwake(s);
  s++;
  if (mask & SELECT_EX)
    selwake(s);
}

static int pipesel_begin(inoptr i, uint8_t bits)
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

static int pipesel_end(inoptr i)
{
  pipesel--;
  /* Clear out wait masks */
  selwait(i, SELECT_IN|SELECT_OUT|SELECT_EX, 0);
}

void selwake_pipe(inoptr i, uint16_t mask)
{
  if (pipesel)
    selwake_inode(i);
}

#define nfd (uint16_t)udata.u_argn
#define base (uint16_t)udata.u_argn1

int _select(void)
{
  struct inode *iptr;
  uint16_t sumo;
  uint8_t n;
  uint16_t inr = 0, outr = 0, exr = 0;
  /* Second 16bits of each spare for expansion */
  uint16_t in = ugetw(base);
  uint16_t out = ugetw(base+4);
  uint16_t ex = ugetw(base+8);
  
  uint16_t sum = in | out | ex;

  /* Timeout in 1/10th of a second (BSD api mangling done by libc) */
  udata.p_tab->p_timeout = ugetw(base + 12);

  do {
    m = 1;
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
        switch(getmode(ino)) {
        /* Device types that automatically report some ready states */
        case F_BDEV:      
        case F_REG:
          outr |= m;
        case F_DIR:
          inr |=  m;
          break;
        case F_PIPE:
          n = pipesel_begin(ino, n);
          goto setbits;
        case F_CDEV:
          /* If unsupported we report the device as read/write ready */
          if (d_ioctl(ino->c_node.i_addr[0], SELECT_BEGIN, &n) == -1) {
            udata.u_error = 0;
            n = SELECT_IN|SELECT_OUT;
          }
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
      m << = 1;		/* Next fd mask */
    }
    inr &= in;		/* Don't reply with bits not being selected */
    outr &= out;
    exr &= ex;
    sumo = inr | outr | exr;	/* Are we there yet ? */
    if (!sumo && psleep_flags(&udata.p_tab->p_timeout, 0) == -1)
      break;
  }
  while (!sumo && udata.p_tab->p_timeout != 1);
  
  udata.p_tab->p_timeout = 0;

  /* Return the values to user space */
  uputw(inr, base);
  uputw(outr, base + 4);
  uputw(exr, base + 8)

  /* Tell the device less people care, we may want to remove all this
     and just select check. The 0 check we could do instead is cheap */
  m = 1;
  for (i = 0; i < nfd; i++) {
    if (sum & m) {
      ino = getinode(i);
      switch(getmode(ino))
      {
        case F_CDEV:
          d_ioctl(ino->c_node.i_addr[0], SELECT_END, NULL);
          break;
        case F_PIPE:
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
