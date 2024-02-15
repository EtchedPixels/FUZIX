#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>

uaddr_t ramtop = PROGTOP;

/* On idle we spin checking for the terminals. Gives us more responsiveness
   for the polled ports */
void plt_idle(void)
{
  /* We don't want an idle poll and IRQ driven tty poll at the same moment */
  irqflags_t irq = di();
  tty_pollirq(); 
  irqrestore(irq);
}

void plt_interrupt(void)
{
 tty_pollirq();
 timer_interrupt();
}

void plt_reinterrupt(void)
{
 tty_pollirq();
}

/* Nothing to do for the map of init */
