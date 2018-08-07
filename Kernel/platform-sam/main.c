#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>

uint16_t ramtop = PROGTOP;
uint8_t need_resched;

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
  tty_interrupt();
  kbd_interrupt();
  timer_interrupt();
}

void map_init(void)
{
}

void platform_discard(void)
{
}

void pagemap_init(void)
{
 pagemap_add(0x63);	/* Mode 3, U64K low 32K mapped as low 32K */
 pagemap_add(0x73);	/* Mode 3, U64K high 32K mapped as low 32K */
}


uint8_t platform_param(char *p)
{
    used(p);
    return 0;
}

