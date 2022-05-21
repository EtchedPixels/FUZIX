#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <tty.h>
#include <printf.h>
#include <deveth.h>

uint16_t ramtop = PROGTOP;

void plt_idle(void)
{
    __asm
    halt
    __endasm;
}

uint8_t plt_param(char *p)
{
    used(p);
    return 0;
}

__sfr __at 0x00 uart0_status;
__sfr __at 0x01 uart0_data;
__sfr __at 0x28 uart1_status;
__sfr __at 0x29 uart1_data;

extern uint8_t irqwork;

void plt_interrupt(void)
{
    uint8_t st0 = uart0_status;
    uint8_t st1 = uart1_status;
    uint8_t ts = irqwork;
    uint8_t d;

    irqwork = 0;
    if (ts & 0xC0) {
        if (ts & 0x80) { 	/* RX data */
            d = uart0_data;
            tty_inproc(1, d);
        }
        if (ts & 0x40)		/* TX idle */
            tty_outproc(1);
    }
    if (st1 & 0xC0) {
        uart1_status = st1 & 0xFC;
        if (st1 & 0x80) { 	/* RX data */
            d = uart1_data;
            tty_inproc(2, d);
        }
        if (st1 & 0x40)		/* TX idle */
            tty_outproc(2);
    }
    if (ts & 1) {
        eth_poll();
        timer_interrupt();
    }
}

void pagemap_init(void)
{
    int i;
    /* 0/1/2 kernel, 3 initial common, 4+ to use */
    for (i = 4; i < 128 ; i++)
        pagemap_add(i);
    /*
     * The kernel boots with 3 as the common, list it last here so it also
     * gets given to init as the kernel kicks off the init stub. init will then
     * exec preserving this common and all forks will be copies from it.
     */
    pagemap_add(3);
}

void map_init(void)
{
}
