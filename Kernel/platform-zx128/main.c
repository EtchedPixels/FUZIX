#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <devmdv.h>

uint16_t ramtop = PROGTOP;

void pagemap_init(void)
{
#ifdef SWAPDEV
  /* Swap */
  swapmap_add(0);
  swapmap_add(1);
  swapmap_add(2);
#endif
}

/* On idle we spin checking for the terminals. Gives us more responsiveness
   for the polled ports */
void platform_idle(void)
{
  /* We don't want an idle poll and IRQ driven tty poll at the same moment */
  irqflags_t irq = di();
  tty_pollirq(); 
  irqrestore(irq);
}

uint8_t platform_param(unsigned char *p)
{
    used(p);
    return 0;
}

void platform_interrupt(void)
{
 tty_pollirq();
 mdv_timer();
 timer_interrupt();
}

/* Nothing to do for the map of init */
void map_init(void)
{
}

#ifndef SWAPDEV
/* Adding dummy swapper since it is referenced by tricks.s */
void swapper(ptptr p)
{
  p;
}
#endif