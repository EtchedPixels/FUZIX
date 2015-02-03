#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <tty.h>
#include <devtty.h>
#include "config.h"
#include <z180.h>

char tbuf1[TTYSIZ];
char tbuf2[TTYSIZ];
char tbuf3[TTYSIZ];
char tbuf4[TTYSIZ];

struct  s_queue  ttyinq[NUM_DEV_TTY+1] = {       /* ttyinq[0] is never used */
    {   NULL,    NULL,    NULL,    0,        0,       0    },
    {   tbuf1,   tbuf1,   tbuf1,   TTYSIZ,   0,   TTYSIZ/2 },
    {   tbuf2,   tbuf2,   tbuf2,   TTYSIZ,   0,   TTYSIZ/2 },
    {   tbuf3,   tbuf2,   tbuf2,   TTYSIZ,   0,   TTYSIZ/2 },
    {   tbuf4,   tbuf2,   tbuf2,   TTYSIZ,   0,   TTYSIZ/2 },
};

void tty_setup(uint8_t minor)
{
    minor;
}

/* For the moment */
int tty_carrier(uint8_t minor)
{
    minor;
    return 1;
}

void tty_pollirq_asci0(void)
{
    while(ASCI_STAT0 & 0x80){
        tty_inproc(1, ASCI_RDR0);
    }
}

void tty_pollirq_asci1(void)
{
    while(ASCI_STAT1 & 0x80){
        tty_inproc(2, ASCI_RDR1);
    }
}

void tty_pollirq_escc(void)
{
    unsigned char rr3;
    ESCC_CTRL_A = 0x03; /* select read register 3 */
    rr3 = ESCC_CTRL_A;

    if(rr3 & 0x20){ /* channel A RX pending */
        tty_inproc(1, ESCC_DATA_A);
    }
    if(rr3 & 0x04){ /* channel B RX pending */
        tty_inproc(2, ESCC_DATA_B);
    }
    if(rr3 & (~0x24)){
        kputs("[escc hurts]");
    }

    ESCC_CTRL_A = 0x38; /* reset interrupt under service */
}

void tty_putc(uint8_t minor, unsigned char c)
{
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
    }
}

ttyready_t tty_writeready(uint8_t minor)
{
    minor;
    return TTY_READY_NOW;
}

/* kernel writes to system console -- never sleep! */
void kputchar(char c)
{
    tty_putc(1, c);
    if(c == '\n')
        tty_putc(1, '\r');
}
