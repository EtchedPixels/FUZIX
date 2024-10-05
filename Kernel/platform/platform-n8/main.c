#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include "config.h"
#include <z180.h>
#include <ps2kbd.h>
#include <ps2mouse.h>
#include <devfd.h>
#include "n8.h"

uint16_t ramtop = PROGTOP;
uint16_t swap_dev = 0xFFFF;
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

    timer_interrupt();
}

void plt_idle(void)
{
    /* Let's go to sleep while we wait for something to interrupt us */
    /* Probably want to change this once we have PS/2 support in */
    __asm
        halt
    __endasm;
}

void plt_interrupt(void)
{
    static uint8_t c;
    uint8_t dummy;
    switch(irqvector){
        case Z180_INT_TIMER0:
            z180_timer_interrupt(); 
            if (!ps2busy) {
                int16_t n;
                if (kbd_open) {
                    n = ps2kbd_get();
                    if (n >= 0)
                        ps2kbd_byte(n);
                }
                if (ps2m_open) {
                    n = ps2mouse_get();
                    if (n >= 0)
                        ps2mouse_byte(n);
                }
            }
#ifdef CONFIG_FLOPPY
            c ^= 1;
            if (c)
                fd_tick();
#endif
            return;
        case Z180_INT_ASCI0:
            tty_pollirq_asci0();
            return;
        case Z180_INT_ASCI1:
            tty_pollirq_asci1();
            return;
        default:
            dummy = tms9918a_ctrl;
            return;
    }
}

/* needs adding for the AY-3-8910 */
void do_beep(void)
{
}
