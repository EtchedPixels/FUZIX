#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>

uint16_t ramtop = PROGTOP;
uint16_t swap_dev;

void platform_idle(void)
{
 __asm
  halt
 __endasm;
}

void platform_interrupt(void)
{
 tty_pollirq();
//FIXME floppy_timer();
 timer_interrupt();
}

/* Nothing to do for the map of init */
void map_init(void)
{
}
