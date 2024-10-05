#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <rtc.h>
#include <lobo.h>

uint16_t ramtop = PROGTOP;
uint8_t vtattr_cap;
uint16_t swap_dev = 0xFFFF;

struct blkbuf *bufpool_end = bufpool + NBUFS;

void do_beep(void)
{
  lobo_io[0x7F8] = 1;
}

uint_fast8_t plt_param(char *p)
{
    used(p);
    return 0;
}

void plt_interrupt(void)
{
  tty_interrupt();
  if (lobo_io[0x7E0] & 0x80) {
    kbd_interrupt();
    timer_interrupt();
  }
}

/*
 *	Once we are about to load init we can throw the boot code away
 *	and convert it into disk cache. This gets us 7 or so buffer
 *	back which more than doubles our cache size !
 */
void plt_discard(void)
{
#if 0
// TODO
  uint16_t discard_size;
  bufptr bp = bufpool_end;

  discard_size /= sizeof(struct blkbuf);

  kprintf("%d buffers reclaimed from discard\n", discard_size);

  bufpool_end += discard_size;	/* Reclaim the discard space */

  memset(bp, 0, discard_size * sizeof(struct blkbuf));
  /* discard_size is in discard so it dies here */
  for (bp = bufpool + NBUFS; bp < bufpool_end; ++bp) {
    bp->bf_dev = NO_DEVICE;
    bp->bf_busy = BF_FREE;
  }
#endif  
}

#ifdef CONFIG_RTC

/* FIXME: timing rules */
uint8_t rtc_readb(uint8_t r)
{
  lobo_io[0x7FE] = r;	/* Read reg 1 */
  lobo_io[0x7FE] = 0x20 | r;
  r = lobo_io[0x7FC];
  lobo_io[0x7FE] = 0x00;
  return r & 0x0F;
}

uint8_t rtc_read2(uint8_t r)
{
  uint8_t v = rtc_readb(r + 1) * 10;
  return v + rtc_readb(r);
}

uint_fast8_t plt_rtc_secs(void)
{
  return rtc_read2(0);
}

/* If the compiler segfaults here you need at least SDCC #10471 */

int plt_rtc_read(void)
{
    uint16_t len = sizeof(struct cmos_rtc);
    struct cmos_rtc cmos;
    uint8_t *p;
    uint8_t r, y;

    if (udata.u_count < len)
        len = udata.u_count;

    /* We do a full set of reads and if the seconds change retry - we
       need to retry the lost as we might read as the second changes for
       new year */
    do {
      p = cmos.data.bytes;
      r = rtc_read2(0);
      y = rtc_read2(11);
      if (y >= 0x70)
          *p++ = 0x19;
      else
          *p++ = 0x20;
      *p++ = y;
      *p++ = rtc_read2(9);
      *p++ = rtc_read2(7);
      *p++ = rtc_read2(4);
      *p++ = rtc_read2(2);
      *p++ = r;
    } while(r != rtc_read2(0));

    cmos.type = CMOS_RTC_BCD;
    if (uput(&cmos, udata.u_base, len) == -1)
        return -1;
    return len;
}

/* Yes I'm a slacker .. this wants adding but it's ugly
   because the seconds is always just set to 0 on any change. We
   also need to deal with leap years here */
int plt_rtc_write(void)
{
	udata.u_error = EOPNOTSUPP;
	return -1;
}

#endif

