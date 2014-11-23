#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>

/* On idle we spin checking for the terminals. Gives us more responsiveness
   for the polled ports */
void platform_idle(void)
{
  tty_pollirq(); 
}

/* The simple support does all the rest of the work for us */

void platform_interrupt(void)
{
 tty_pollirq();
 timer_interrupt();
}

/* Nothing to do for the map of init */
void map_init(void)
{
}
