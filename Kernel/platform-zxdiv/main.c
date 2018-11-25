#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>

uint16_t ramtop = PROGTOP;
uint16_t swap_dev = 0xFFFF;

/* On idle we spin checking for the terminals. Gives us more responsiveness
   for the polled ports */
void platform_idle(void)
{
  /* We don't want an idle poll and IRQ driven tty poll at the same moment */
  __asm
   halt
  __endasm;
}

void platform_interrupt(void)
{
 tty_pollirq();
 timer_interrupt();
}

/*
 *	So that we don't suck in a library routine we can't use from
 *	the runtime
 */

int strlen(const char *p)
{
  int len = 0;
  while(*p++)
    len++;
  return len;
}

#ifndef SWAPDEV
/* Adding dummy swapper since it is referenced by tricks.s */
void swapper(ptptr p)
{
  p;
}
#endif