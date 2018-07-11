#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <rtc.h>
#include <devtty.h>
#include <trs80.h>

uint16_t ramtop = PROGTOP;
uint8_t trs80_model;
uint8_t vtattr_cap;

struct blkbuf *bufpool_end = bufpool + NBUFS;

/*
 *	Called when there is no work to do. On the models without serial
 *	interrupts we poll here so that the normal case of idling while
 *	waiting for input feels ok.
 */
void platform_idle(void)
{
  irqflags_t irq;
  /* The Model III has a real interrupt driven serial port */
  if (trs80_model == TRS80_MODEL3) {
    __asm
      halt
    __endasm;
    return;
  }
  /* The others .. do not. For the model I and LNW80 we just poll the
     port as if it interrupted, for the Video Genie we have a different
     helper */
  irq = di();
  if (trs80_model == VIDEOGENIE)
    tty_vg_poll();
  else
    tty_interrupt();
  irqrestore(irq);
}

void do_beep(void)
{
}

uint8_t platform_param(char *p)
{
    used(p);
    return 0;
}

/* Work around SDCC bugs */
uint8_t sdcc_bug_2753(uint8_t v) __z88dk_fastcall
{
  return v;
}

__sfr __at 0xE0 irqstat3;
__sfr __at 0xEC irqack3;

/* We assign these to dummy to deal with an sdcc bug (should be fixed in next
   SDCC) */
void platform_interrupt(void)
{
  uint8_t dummy;
  if (trs80_model != TRS80_MODEL3) {
    uint8_t irq = *((volatile uint8_t *)0x37E0);

    tty_interrupt();
    kbd_interrupt();

    if (irq & 0x40)
      dummy = sdcc_bug_2753(*((volatile uint8_t *)0x37EC));
    if (irq & 0x80) {	/* FIXME??? */
      timer_interrupt();
      dummy = sdcc_bug_2753(*((volatile uint8_t *)0x37E0));	/* Ack the timer */
    }
  } else {
    /* The Model III IRQ handling has to be different... */
    uint8_t irq = ~irqstat3;
    /* Serial port ? */
    if (irq & 0x70)
      tty_interrupt();
    /* We don't handle IOBUS */
    if (irq & 0x04) {
      kbd_interrupt();
      timer_interrupt();
      dummy = irqack3;
    }
  }
}

/*
 *	We can't recover discard space usefully... yet. I have a cunning plan
 *	involving external buffers in the spare bank space 8)
 */

void platform_discard(void)
{
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
uint8_t platform_rtc_secs(void)
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

int platform_rtc_read(void)
{
#if 0
    /* We need SDCC bug 2770 fixed first */
    uint16_t len;
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
      if (y > 70)
          *p++ = 19;
      else
          *p++ = 20;
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
#else
	udata.u_error = EOPNOTSUPP;
	return -1;
#endif    
}

/* Yes I'm a slacker .. this wants adding but it's ugly
   because the seconds is always just set to 0 on any change. We
   also need to deal with leap years here */
int platform_rtc_write(void)
{
	udata.u_error = EOPNOTSUPP;
	return -1;
}

#endif

/*
 *	So that we don't suck in a library routine we can't use from
 *	the runtime
 */

int strlen(const char *p)
{
  int len = 0;
  while(*p++)
    len++;
  return len;
}
