#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>

uint16_t ramtop = PROGTOP;
uint16_t vdpport = 0x01 + 256 * 40;	/* port and width */

void pagemap_init(void)
{
 int i;
 /* Ten banks (should check memory size) FIXME */
 for (i = 0x81; i < 0x8A; i++)
  pagemap_add(i);
}

/* On idle we spin checking for the terminals. Gives us more responsiveness
   for the polled ports */
void platform_idle(void)
{
  /* We don't want an idle poll and IRQ driven tty poll at the same moment */
  irqflags_t irq = di();
//  tty_pollirq(); 
  irqrestore(irq);
}

void platform_interrupt(void)
{
 kbd_interrupt();
 timer_interrupt();
}

/* Nothing to do for the map of init */
void map_init(void)
{
}

void do_beep(void)
{
}
