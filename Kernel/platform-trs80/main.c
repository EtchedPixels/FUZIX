#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <rtc.h>

uint16_t ramtop = PROGTOP;
__sfr __at 0xE0 irqstat;
__sfr __at 0xEF irqack;

uint8_t banktype;
uint8_t vtattr_cap;

struct blkbuf *bufpool_end = bufpool + NBUFS;

void plt_idle(void)
{
    __asm
    halt
    __endasm;
}

void do_beep(void)
{
}

uint8_t plt_param(char *p)
{
    used(p);
    return 0;
}

void plt_interrupt(void)
{
  uint8_t irq = ~irqstat;
  uint8_t dummy;
  if (irq & 0x20)
    tty_interrupt();
  if (irq & 0x04) {
    kbd_interrupt();
    timer_interrupt();
    dummy = irqack;
  }
}

/*
 *	Once we are about to load init we can throw the boot code away
 *	and convert it into disk cache. This gets us 7 or so buffer
 *	back which more than doubles our cache size !
 */
void plt_discard(void)
{
  extern uint16_t discard_size;
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
}

#ifdef CONFIG_RTC

__sfr __at 0xB0 rtc_secl;
__sfr __at 0xB1 rtc_sech;
__sfr __at 0xB2 rtc_minl;
__sfr __at 0xB3 rtc_minh;
__sfr __at 0xB4 rtc_hourl;
__sfr __at 0xB5 rtc_hourh;
/* day of week is B6 */
__sfr __at 0xB7 rtc_dayl;
__sfr __at 0xB8 rtc_dayh;
__sfr __at 0xB9 rtc_monl;
__sfr __at 0xBA rtc_monh;
__sfr __at 0xBB rtc_yearl;
__sfr __at 0xBC rtc_yearh;

/* FIXME: the RTC is optional so we should test for it first */
uint8_t plt_rtc_secs(void)
{
    uint8_t sl, rv;
    /* BCD encoded */
    do {
        sl = rtc_secl;
        /* RTC may be absent */
        if (sl == 255)
          return 255;
        rv = sl + rtc_sech * 10;
    } while (sl != rtc_secl);
    return rv;
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

    if (rtc_secl == 255) {
      udata.u_error = EOPNOTSUPP;
      return -1;
    }

    /* We do a full set of reads and if the seconds change retry - we
       need to retry the lost as we might read as the second changes for
       new year */
    do {
      p = cmos.data.bytes;
      r = rtc_secl;
      y  = (rtc_yearh << 4) | rtc_yearl;
      if (y >= 0x70)
          *p++ = 0x19;
      else
          *p++ = 0x20;
      *p++ = y;
      *p++ = ((rtc_monh  & 1)<< 4) | rtc_monl;
      *p++ = ((rtc_dayh & 3) << 4) | rtc_dayl;
      *p++ = ((rtc_hourh & 3) << 4) | rtc_hourl;
      *p++ = ((rtc_minh & 7) << 4) | rtc_minl;
      *p++ = ((rtc_sech & 7) << 4) | rtc_secl;
    } while ((r ^ rtc_secl) & 0x0F);

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

