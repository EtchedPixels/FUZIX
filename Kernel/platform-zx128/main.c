#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <devmdv.h>

uint16_t ramtop = PROGTOP;

void pagemap_init(void)
{
  /* The live process also has 2 and the non running one 6 */
  pagemap_add(4);
  pagemap_add(3);
}

/* On idle we spin checking for the terminals. Gives us more responsiveness
   for the polled ports */
void platform_idle(void)
{
 __asm
  halt
 __endasm;
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
