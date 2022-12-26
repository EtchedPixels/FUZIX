#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <devinput.h>

uint16_t ramtop = PROGTOP;

/* On idle we spin checking for the terminals. Gives us more responsiveness
   for the polled ports */
void plt_idle(void)
{
  /* We don't want an idle poll and IRQ driven tty poll at the same moment */
  __asm
   halt
  __endasm;
}

uint8_t timer_wait;

void plt_interrupt(void)
{
 tty_pollirq();
 timer_interrupt();
 poll_input();
 if (timer_wait)
  wakeup(&timer_interrupt);
}

/*
 *	So that we don't suck in a library routine we can't use from
 *	the runtime
 */

size_t strlen(const char *p)
{
  size_t len = 0;
  while(*p++)
    len++;
  return len;
}

/*
 *	We pack discard into the memory image is if it were just normal
 *	code but place it at the end after the buffers. When we finish up
 *	booting we turn everything from the buffer pool to the start of
 *	user space into buffers.
 *
 *	We don't touch discard. Discard is just turned into user space.
 */
void plt_discard(void)
{
}

#ifndef SWAPDEV
/* Adding dummy swapper since it is referenced by tricks.s */
void swapper(ptptr p)
{
  p;
}
#endif
