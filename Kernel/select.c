#include <kernel.h>
#include <version.h>
#include <kdata.h>
#include <printf.h>

/*
 *	Logic for select
 *
 *	Q: where to hide pipe selmaps ?
 *
 *	Could have a generic selmap set for 'all pipes' - ugly but keeps
 *	costs down.
 */

void seladdwait(struct selmap *s)
{
  int16_t p = udata.u_ptab - ptab;
  s->map[p>>3] |= (1 << (p & 7));
}

void selrmwait(struct selmap *s)
{
  int16_t p = udata.u_ptab - ptab;
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
        ino = getinode(i);
        if (ino == NULLINODE)
          return -1;
        switch(getmode(ino)) {
        case F_BDEV:      
        case F_REG:
          outr |= m;
        case F_DIR:
          inr |=  m;
          break;
        case F_PIPE:
          /* TODO */
          break;
        case F_CDEV:
          /* Not fatal: counts as read/write ready */
          if (d_ioctl(ino->c_node.i_addr[0], SELECT_BEGIN, &n) == -1) {
            udata.u_error = 0;
            n = SELECT_IN|SELECT_OUT;
          }
          if (n & SELECT_IN)
            inr |= m;
          if (n & SELECT_OUT)
            outr |= m;
          if (n & SELECT_EX)
            exr |= m;
          break;
      }
    }
    m << = 1;
    sumo = inr | outr | exr;
    if (psleep_flags(&udata.p_tab->p_timeout, 0) == -1)
      break;
  }
  while (!sumo && udata.p_tab->p_timeout != 1);
  
  /* FIXME: write back properly */
  *inp = inr;
  *outp = outr;
  *exp = exr;

  m = 1;
  for (i = 0; i < nfd; i++) {
    if (sum & m) {
      ino = getinode(i);
      if (getmode(ino) == F_CDEV)
        d_ioctl(ino->c_node.i_addr[0], SELECT_END, NULL);
      /* FIXME - pipe */
    }
    m <<= 1;
  }
  return 0;		/* the scan and return of highest fd is done by
                           user space */
}

#undef nfd
#undef base
