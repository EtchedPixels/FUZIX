#define _DEVTTY_PRIVATE
#include <kernel.h>
#include "config.h"
#include <z180.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <tty.h>
#include <devtty.h>

/* Will Sowerbutts 2015-02-15
 *
 * The P112 has so many serial ports! Here's a quick map:
 *
 * tty1 -- ESCC channel A (P4 header, RS232 levels, boot ROM uses this port)
 * tty2 -- ESCC channel B (P14 header, TTL levels)
 * tty3 -- ASCI channel 0 (P14 header, TTL levels)
 * tty4 -- ASCI channel 1 (P14 header, TTL levels)
 * tty5 -- SMC 16550 COM1 (P8 header, RS232 levels)
 *
 * We assume RTS/CTS flow control is in use.
 *
 * Note that the SMC has a second 16550 (COM2) but the pins are
 * not presented on a header ... I guess five was enough!


 * A note on interrupts:
 *
 * In normal operation we configure UART to generate an interrupt only when
 * data has been received.
 *
 * When a transmitting process goes to sleep waiting on the UART (either
 * because CTS is low or because it's time to reschedule) we enable interrupts
 * on both CTS status change and transmitter ready. These are then disabled
 * when we re-awaken the process. This has to be done with ints disabled
 * otherwise there is a race to put the process to sleep before the interrupt
 * to awaken it arrives.
 */

/* TODO:
 *  - only do RTS/CTS flow control when configured to do so (how to test?)
 *  - drive RTS signals, requires tty_inproc() to signal to us when to do so
 *  - resurrect tty_carrier() code?
 *  - implement RTS/CTS for ASCI (waiting on my making up a cable ...)
 */

static uint8_t tbuf1[TTYSIZ];
static uint8_t tbuf2[TTYSIZ];
static uint8_t tbuf3[TTYSIZ];
static uint8_t tbuf4[TTYSIZ];
static uint8_t tbuf5[TTYSIZ];

struct  s_queue  ttyinq[NUM_DEV_TTY+1] = {       /* ttyinq[0] is never used */
    {   NULL,    NULL,    NULL,    0,        0,       0    },
    {   tbuf1,   tbuf1,   tbuf1,   TTYSIZ,   0,   TTYSIZ/2 },
    {   tbuf2,   tbuf2,   tbuf2,   TTYSIZ,   0,   TTYSIZ/2 },
    {   tbuf3,   tbuf3,   tbuf3,   TTYSIZ,   0,   TTYSIZ/2 },
    {   tbuf4,   tbuf4,   tbuf4,   TTYSIZ,   0,   TTYSIZ/2 },
    {   tbuf5,   tbuf5,   tbuf5,   TTYSIZ,   0,   TTYSIZ/2 },
};

/* TODO: stty support on the ESCC, ASCI and 16550 */
tcflag_t termios_mask[NUM_DEV_TTY + 1] = {
	0,
	_CSYS,
	_CSYS,
	_CSYS,
	_CSYS,
	_CSYS
};

/* tty_hw_init() which sets up tty5 can be found in discard.c */

void tty_setup(uint_fast8_t minor, uint_fast8_t flags)
{
    minor;
}

int tty_carrier(uint_fast8_t minor)
{
#if 0   /* The code below works -- but if ESCC A has DCD low on boot
           the system crashes, which is no fun. So we just lie and 
           always report that it is high. A battle for another day. */
    uint8_t c;

    switch(minor){
        /* ---- ESCC ports ------------------------- */
        case 1:
            c = ESCC_CTRL_A;
escc_carriertest:
            return (c & 8) ? 1 : 0; /* test DCD */
        case 2:
            c = ESCC_CTRL_B;
            goto escc_carriertest;
        /* ---- 16550 port ------------------------- */
        case 5:
            c = TTY_COM1_MSR;
            return (c & 0x80) ? 1 : 0; /* test DCD */

        /* ---- ASCI ports ------------------------- */
            /* TODO: test DCD for ASCI */
    }
#endif
    minor; /* unused */
    return 1;
}

void tty_pollirq_asci0(void)
{
    while(ASCI_STAT0 & 0x80){
        tty_inproc(3, ASCI_RDR0);
    }
    if (ASCI_STAT0 & 0x70)
        ASCI_CNTLA0 &= ~0x08;
}

void tty_pollirq_asci1(void)
{
    while(ASCI_STAT1 & 0x80){
        tty_inproc(4, ASCI_RDR1);
    }
    if (ASCI_STAT1 & 0x70)
        ASCI_CNTLA1 &= ~0x08;
}

void tty_pollirq_com1(void)
{
    uint8_t iir, msr, lsr;

    while(true){
        iir = TTY_COM1_IIR;
        lsr = TTY_COM1_LSR;

        /* IIR bits
         * 3 2 1 0
         * -------
         * x x x 1     no interrupt pending
         * 0 1 1 0  6  LSR changed -- read the LSR
         * 0 1 0 0  4  receive FIFO >= threshold
         * 1 1 0 0  C  received data sat in FIFO for a while
         * 0 0 1 0  2  transmit holding register empty
         * 0 0 0 0  0  MSR changed -- read the MSR
         */
        switch(iir & 0x0F){
            case 0x0: /* MSR changed */
            case 0x2: /* transmit register empty */
                /* TODO: with DCD implemented we need to spot DCD changes here also */
                msr = TTY_COM1_MSR;
                if((msr & 0x10) && (lsr & 0x20)){ /* CTS high, transmit reg empty */
                    tty_outproc(5);
                }
                /* fall through */
            case 0x6: /* LSR changed */
                /* we already read the LSR register so int has cleared */
                TTY_COM1_IER = 0x01; /* enable only receive interrupts */
                break;
            case 0x4: /* receive (FIFO >= threshold) */
            case 0xC: /* receive (timeout waiting for FIFO to fill) */
                while(lsr & 0x01){ /* Data ready */
                    tty_inproc(5, TTY_COM1_RBR);
                    lsr = TTY_COM1_LSR;
                }
                break;
            default:
                return;
        }
    }
}

void tty_pollirq_escc(void)
{
    uint8_t rr3;

    ESCC_CTRL_A = 0x03; /* select read register 3 */
    rr3 = ESCC_CTRL_A;

    if(rr3 & 0x20){ /* channel A RX pending */
        while(ESCC_CTRL_A & 1)
            tty_inproc(1, ESCC_DATA_A);
    }

    if(rr3 & 0x04){ /* channel B RX pending */
        while(ESCC_CTRL_B & 1)
            tty_inproc(2, ESCC_DATA_B);
    }

    if(rr3 & 0x18){ /* channel A TX pending or ext/status */
        if(rr3 & 0x10)
            ESCC_CTRL_A = 0x28; /* reset transmit interrupt */
        if(rr3 & 0x08)
            ESCC_CTRL_A = 0x10; /* reset external/status interrupts */
        /* disable further tx/status ints */
        ESCC_CTRL_A = 0x01; /* select WR1 */
        ESCC_CTRL_A = 0x10; /* receive interrupts only please */
        ESCC_CTRL_A = 0x0F; /* select WR15 */
        ESCC_CTRL_A = 0x01; /* disable CTS interrupts (otherwise it latches CTS in RR0) */
        tty_outproc(1);
    }

    if(rr3 & 0x03){ /* channel B TX pending or ext/status */
        if(rr3 & 0x02)
            ESCC_CTRL_B = 0x28; /* reset transmit interrupt */
        if(rr3 & 0x01)
            ESCC_CTRL_B = 0x10; /* reset external/status interrupts */
        /* disable further tx/status ints */
        ESCC_CTRL_B = 0x01; /* select WR1 */
        ESCC_CTRL_B = 0x10; /* receive interrupts only please */
        ESCC_CTRL_B = 0x0F; /* select WR15 */
        ESCC_CTRL_B = 0x01; /* disable CTS interrupts (otherwise it latches CTS in RR0) */
        tty_outproc(2);
    }

    ESCC_CTRL_A = 0x38; /* reset interrupt under service */
}

void tty_sleeping(uint_fast8_t minor)
{
    /* enable tx/status ints so we can awaken the process */
    switch(minor){
        case 1:
            ESCC_CTRL_A = 0x01; /* select WR1 */
            ESCC_CTRL_A = 0x17; /* receive, transmit, ext/status interrupts */
            ESCC_CTRL_A = 0x0F; /* select WR15 */
            ESCC_CTRL_A = 0x21; /* enable CTS interrupts */
            break;
        case 2:
            ESCC_CTRL_B = 0x01; /* select WR1 */
            ESCC_CTRL_B = 0x17; /* receive, transmit, ext/status interrupts */
            ESCC_CTRL_B = 0x0F; /* select WR15 */
            ESCC_CTRL_B = 0x21; /* enable CTS interrupts */
            break;
        case 5:
            TTY_COM1_IER = 0x0B; /* enable all but LSR interrupt */
    }
}

ttyready_t tty_writeready(uint_fast8_t minor)
{
    uint8_t c;

    switch(minor){
        default:
            return TTY_READY_NOW;

        /* ---- ESCC ports ------------------------- */
        case 1:
            c = ESCC_CTRL_A;
escc_readytest:
            if((c & 0x20) == 0) /* CTS not asserted? */
                return TTY_READY_LATER;
            else if(c & 0x04) /* Transmit empty? */
                return TTY_READY_NOW;
            else /* This could be made baud rate dependent */
                return TTY_READY_SOON;

        case 2:
            c = ESCC_CTRL_B;
            goto escc_readytest;

        /* ---- ASCI ports ------------------------- */
        case 3:
            c = ASCI_STAT0;
asci_readytest:
            if(c & 2)
                return TTY_READY_NOW;
            return TTY_READY_SOON;

        case 4:
            c = ASCI_STAT1;
            goto asci_readytest;

        /* ---- 16550 port ------------------------- */
        case 5:
            c = TTY_COM1_MSR;
            if((c & 0x10) == 0) /* CTS not asserted? */
                return TTY_READY_LATER;
            c = TTY_COM1_LSR;
            if( c & 0x20 ) /* THRE? */
                return TTY_READY_NOW;
            return TTY_READY_SOON;
    }
}

void tty_putc(uint_fast8_t minor, uint_fast8_t c)
{
    /* note that these all ignore CTS; tty_writeready checks it, but it does
     * mean kernel writes to console ignore flow control.  This could be easily
     * fixed, but then we could get stuck in here indefinitely if CTS drops
     * between tty_writeready and tty_putc */

    switch(minor){
        case 1:
            while(!(ESCC_CTRL_A & 4));
            ESCC_DATA_A = c;
            break;
        case 2:
            while(!(ESCC_CTRL_B & 4));
            ESCC_DATA_B = c;
            break;
        case 3:
            while(!(ASCI_STAT0 & 2));
            ASCI_TDR0 = c;
            break;
        case 4:
            while(!(ASCI_STAT1 & 2));
            ASCI_TDR1 = c;
            break;
        case 5:
            while(!(TTY_COM1_LSR & 0x20));
            TTY_COM1_THR = c;
            break;
    }
}

/* kernel writes to system console -- never sleep! */
void kputchar(uint_fast8_t c)
{
    tty_putc(TTYDEV & 0xFF, c);
    if(c == '\n')
        tty_putc(TTYDEV & 0xFF, '\r');
}

void tty_data_consumed(uint_fast8_t minor)
{
}
