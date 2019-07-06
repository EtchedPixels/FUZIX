#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <blkdev.h>
#include <devfdc765.h>

uaddr_t ramtop = PROGTOP;
uint16_t swap_dev = 0xFFFF;

extern uint16_t probe_bank(uint16_t);


/* On idle we spin checking for the terminals. Gives us more responsiveness
   for the polled ports */
void platform_idle(void)
{
  /* We don't want an idle poll and IRQ driven tty poll at the same moment */
  irqflags_t irq = di();
  tty_poll(); 
  irqrestore(irq);
}

void platform_interrupt(void)
{
 tty_poll();
 timer_interrupt();
 devfd_spindown();
}

