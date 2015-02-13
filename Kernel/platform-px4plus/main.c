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
    __asm
    halt
    __endasm;
}

__sfr __at 0x04	isr;

void platform_interrupt(void)
{
 uint8_t irq = isr;
 /* We need to handle 1 for the 7508, and 2 for GAPNIO (serial in) at least */
 /* Overflow on timer */
 if (irq & 4)
  timer_interrupt();
}

/* Nothing to do for the map of init */
void map_init(void)
{
}

