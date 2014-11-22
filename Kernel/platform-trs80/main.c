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
  tty_pollirq(); 
}

void do_beep(void)
{
}

void platform_interrupt(void)
{
  timer_interrupt();
}

void map_init(void)
{
}
