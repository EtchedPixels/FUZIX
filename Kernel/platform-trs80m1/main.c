#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
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
