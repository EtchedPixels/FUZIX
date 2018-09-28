#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>

/* These are set by the msx startup asm code */
extern uint16_t vdpport;
uint16_t infobits;
uint16_t swap_dev = 0xFFFF;
uint16_t ramtop = 0xC000;
uint8_t machine_type;
uint8_t vdptype;

void platform_idle(void)
{
    __asm
    halt
    __endasm;
}

/* Some of this is discard stuff */

uint8_t platform_param(char *p)
{
    used(p);
    return 0;
}

void do_beep(void)
{
}

void platform_interrupt(void)
{
        uint8_t r = in((uint8_t)vdpport);
        if (r & 0x80) {
  	  kbd_interrupt();
  	  timer_interrupt();
        }
}

