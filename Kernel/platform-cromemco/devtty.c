#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <tty.h>
#include <devtty.h>
#include <irq.h>

static char tbuf1[TTYSIZ];
static char tbuf2[TTYSIZ];
static char tbuf3[TTYSIZ];

struct  s_queue  ttyinq[NUM_DEV_TTY+1] = {       /* ttyinq[0] is never used */
    {   NULL,    NULL,    NULL,    0,        0,       0    },
    {   tbuf1,   tbuf1,   tbuf1,   TTYSIZ,   0,   TTYSIZ/2 },
    {   tbuf2,   tbuf2,   tbuf2,   TTYSIZ,   0,   TTYSIZ/2 },
    {   tbuf3,   tbuf3,   tbuf3,   TTYSIZ,   0,   TTYSIZ/2 },
};

tcflag_t termios_mask[NUM_DEV_TTY + 1] = {
	0,
	CBAUD|CSTOPB|_CSYS,
	CBAUD|CSTOPB|_CSYS,
	CBAUD|CSTOPB|_CSYS
};

static uint8_t ttypoll;

static uint8_t ttybase[] = { 0, 0, 32, 80 };

uint8_t tty_irqmode = 0;

__sfr __at 0x00 tuart0_status;
__sfr __at 0x01 tuart0_data;

/* Write to system console */
void kputchar(char c)
{
    irqflags_t irq;
    if (c == '\n')
        kputchar('\r');
    /* FIXME: spins in irq on */
    while(1) {
        /* Spin with interrupts on if possible */
        while(!(tuart0_status & 0x80));
        irq = di();
        if (tuart0_status & 0x80) {
            tuart0_data = c;
            irqrestore(irq);
            return;
        }
        /* Raced with the tty driver so try again */
        irqrestore(irq);
    }
}

char tty_writeready(uint8_t minor)
{
    if (minor == 1 && tuart0_txl < 63)
        return TTY_READY_NOW;
    if (minor == 2 && tuart1_txl < 63)
        return TTY_READY_NOW;
    if (minor == 3 && tuart2_txl < 63)
        return TTY_READY_NOW;
    return TTY_READY_SOON;
}

void tty_putc(uint8_t minor, unsigned char c)
{
    irqflags_t irq;
    if (tty_writeready(minor) != TTY_READY_NOW)
        return;
    irq = di();
    if (minor == 1)
        tuart0_txqueue(c);
    else if (minor == 2)
        tuart1_txqueue(c);
    else
        tuart2_txqueue(c);
    irqrestore(irq);
}

void tty_sleeping(uint8_t minor)
{
    ttypoll |= 1 << minor;
}

void tty_data_consumed(uint8_t minor)
{
    used(minor);
}

void tty_drain(void)
{
    while (tuart0_rxl)
        tty_inproc(1, tuart0_rx_get());
    if (tuart0_txl < 32 && (ttypoll & (1 << 1))) {
        ttypoll &= ~(1 << 1);
        tty_outproc(1);
    }
    while (tuart1_rxl)
        tty_inproc(2, tuart1_rx_get());
    if (tuart1_txl < 32 && (ttypoll & (1 << 2))) {
        ttypoll &= ~(1 << 2);
        tty_outproc(2);
    }
    while (tuart2_rxl)
        tty_inproc(3, tuart2_rx_get());
    if (tuart2_txl < 32 && (ttypoll & (1 << 3))) {
        ttypoll &= ~(1 << 3);
        tty_outproc(3);
    }
}    
 
static uint8_t baudbits[] = {
    1,
    1,
    1,
    1,	/* 110 */
    1,
    2,	/* 150 */
    4,	/* 300 */
    4,
    8,	/* 1200 */
    16,	/* 2400 */
    32,	/* 4800 */
    64,	/* 9600 */
    /* x8 modes */
    8,	/* 19200 */
    16,	/* 38400 */
    16,
    16
};

void tty_setup(uint8_t minor, uint8_t flags)
{
    struct termios *t = &ttydata[minor].termios;
    uint8_t baud = t->c_cflag & CBAUD;
    uint8_t r = ttybase[minor];

    used(flags);

    out(r, baudbits[baud] | (t->c_cflag & CSTOPB) ? 0 : 0x80);
    if (baud <= B9600)
        out(r + 2, 0x8);
    else
        out(r + 2, 0x18);
}

int tty_carrier(uint8_t minor)
{
    used(minor);
    return 1;
}
