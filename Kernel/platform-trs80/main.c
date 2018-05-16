#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>

uint16_t ramtop = PROGTOP;
__sfr __at 0xE0 irqstat;
__sfr __at 0xEF irqack;

uint8_t vtattr_cap;

struct blkbuf *bufpool_end = bufpool + NBUFS;

/* On idle we spin checking for the terminals. Gives us more responsiveness
   for the polled ports */
void platform_idle(void)
{
    __asm
    halt
    __endasm;
}

void do_beep(void)
{
}

uint8_t platform_param(char *p)
{
    used(p);
    return 0;
}

void platform_interrupt(void)
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
void platform_discard(void)
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

uint8_t platform_rtc_secs(void)
{
    uint8_t sl, rv;
    /* BCD encoded */
    do {
        sl = rtc_secl;
        rv = sl + rtc_sech * 10;
    } while (sl != rtc_secl);
    return rv;
}

#endif

