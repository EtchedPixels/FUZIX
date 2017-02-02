#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <devfd.h>

uint16_t ramtop = PROGTOP;
uint16_t vdpport = 0x02 + 256 * 40;	/* port and width */
uint8_t membanks;

void pagemap_init(void)
{
 int i;
 /* Up to ten banks */
 for (i = 0x81; i < 0x80 + membanks; i++)
  pagemap_add(i);
}

void platform_idle(void)
{
    __asm
    halt
    __endasm;
}

uint8_t platform_param(char *p)
{
    used(p);
    return 0;
}

void platform_interrupt(void)
{
  extern uint8_t irqvector;

  if (irqvector == 1) {
    tty_interrupt();
    return;
  }
  kbd_interrupt();
  fd_motor_timer();
  timer_interrupt();
}

/* Nothing to do for the map of init */
void map_init(void)
{
}

void do_beep(void)
{
}
