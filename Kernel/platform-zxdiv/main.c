#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>

uint16_t ramtop = PROGTOP;

/* On idle we spin checking for the terminals. Gives us more responsiveness
   for the polled ports */
void platform_idle(void)
{
  /* We don't want an idle poll and IRQ driven tty poll at the same moment */
  irqflags_t irq = di();
  tty_pollirq(); 
  irqrestore(irq);
}

void platform_interrupt(void)
{
 tty_pollirq();
 timer_interrupt();
}

size_t strlcpy(char *dst, const char *src, size_t dstsize)
{
  size_t len = strlen(src);
  size_t cp = len >= dstsize ? dstsize - 1 : len;
  memcpy(dst, src, cp);
  dst[cp] = 0;
  return len;
}

#ifndef SWAPDEV
/* Adding dummy swapper since it is referenced by tricks.s */
void swapper(ptptr p)
{
  p;
}
#endif