#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include "config.h"
#include <z180.h>
#include <bq4845.h>
#include <devfd.h>

uint16_t ramtop = PROGTOP;
extern unsigned char irqvector;
uint16_t swap_dev = 0xFFFF;

uint16_t rtc_port = 0x0C;
uint8_t rtc_shadow;

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
    static uint8_t n;
    unsigned char a;

    /* we have to read both of these registers in order to reset the timer */
    a = TIME_TCR;
    a = TIME_TMDR0L;
    timer_interrupt();
    /* FDC timer */
    n ^= 1;
    if (n)
        fd_tick();
}

void plt_idle(void)
{
    /* Let's go to sleep while we wait for something to interrupt us */
    __asm
        halt
    __endasm;
}

void plt_interrupt(void)
{
    switch(irqvector){
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
            return;
    }
}

void bq4845_write(uint_fast8_t reg, uint_fast8_t val)
{
    out(0x50 + reg, val);
}

uint_fast8_t bq4845_read(uint_fast8_t reg)
{
    return in(0x50 + reg);
}
