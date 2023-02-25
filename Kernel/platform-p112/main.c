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
struct blkbuf *bufpool_end = bufpool + NBUFS; /* minimal for boot -- expanded after we're done with _DISCARD */

void plt_discard(void)
{
    while(bufpool_end < (struct blkbuf*)(KERNTOP - sizeof(struct blkbuf))){
        memset(bufpool_end, 0, sizeof(struct blkbuf));
#if BF_FREE != 0
        bufpool_end->bf_busy = BF_FREE; /* redundant when BF_FREE == 0 */
#endif
        bufpool_end->bf_dev = NO_DEVICE;
        bufpool_end++;
    }
}

void z180_timer_interrupt(void)
{
    unsigned char a;

    /* we have to read both of these registers in order to reset the timer */
    a = TIME_TCR;
    a = TIME_TMDR0L;

    /* FIXME: we are calling this twice the rate it expects */
#ifdef CONFIG_P112_FLOPPY
    fd_tick();
#endif

    timer_interrupt();
}

void plt_idle(void)
{
    /* Let's go to sleep while we wait for something to interrupt us;
     * sadly no fun LED to change colour */
    __asm
        halt
    __endasm;
}

void plt_interrupt(void)
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
