#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>

uint16_t ramtop = PROGTOP;
__sfr __at 0xE0 irqstat;
__sfr __at 0xEF irqack;

/* On idle we spin checking for the terminals. Gives us more responsiveness
   for the polled ports */
void platform_idle(void)
{
    __asm
    halt
    __endasm;
}

void do_beep(void)
{
}

void platform_interrupt(void)
{
  uint8_t irq = irqstat;
  if (irq & 0x20)
    tty_interrupt();
  if (!(irq & 0x80))
    return;
  kbd_interrupt();
  timer_interrupt();
}

#ifdef CONFIG_RTC

__sfr __at 0xB0 rtc_secl;
__sfr __at 0xB1 rtc_sech;

uint8_t rtc_secs(void)
{
    uint8_t sl, rv;
    /* BCD encoded */
    do {
        sl = rtc_secl;
        rv = sl + rtc_sech * 10;
    } while (sl != rtc_secl);
    return rv;
}

#endif

DISCARDABLE

void map_init(void)
{
}

void pagemap_init(void)
{
 pagemap_add(0x63);	/* Mode 3, U64K low 32K mapped as low 32K */
 pagemap_add(0x73);	/* Mode 3, U64K high 32K mapped as low 32K */
}
