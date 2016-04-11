#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <z180.h>
#include "config.h"
#ifdef CONFIG_P112_FLOPPY
#include "devfd.h"
#endif

uint16_t ramtop = PROGTOP;
extern unsigned char irqvector;

void z180_timer_interrupt(void)
{
    unsigned char a;

    /* we have to read both of these registers in order to reset the timer */
    a = TIME_TMDR0L;
    a = TIME_TCR;

#ifdef CONFIG_P112_FLOPPY
    fd_tick();
#endif

    timer_interrupt();
}

void platform_idle(void)
{
    /* Let's go to sleep while we wait for something to interrupt us;
     * sadly no fun LED to change colour */
    __asm
        halt
    __endasm;
}

uint8_t platform_param(unsigned char *p)
{
    used(p);
    return 0;
}

void platform_interrupt(void)
{
    switch(irqvector){
        case Z180_INT0:
            tty_pollirq_escc();
            return;
        case Z180_INT2:
            tty_pollirq_com1();
            return;
        case Z180_INT_TIMER0:
            z180_timer_interrupt(); 
            return;
        case Z180_INT_ASCI0:
            tty_pollirq_asci0();
            return;
        case Z180_INT_ASCI1:
            tty_pollirq_asci1();
            return;
        default:
            kprintf("[int:%d?]", irqvector);
            return;
    }
}
