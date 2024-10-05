#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <tty.h>
#include <devtty.h>

#include "rabbit.h"

static char tbuf1[TTYSIZ];
static char tbuf2[TTYSIZ];
static char tbuf3[TTYSIZ];

struct  s_queue  ttyinq[NUM_DEV_TTY+1] = {       /* ttyinq[0] is never used */
    {   NULL,    NULL,    NULL,    0,        0,       0    },
    {   tbuf1,   tbuf1,   tbuf1,   TTYSIZ,   0,   TTYSIZ/2 },
    {   tbuf2,   tbuf2,   tbuf2,   TTYSIZ,   0,   TTYSIZ/2 },
    {   tbuf3,   tbuf3,   tbuf3,   TTYSIZ,   0,   TTYSIZ/2 },
};

/* Port B is owned by our SPI code */
static uint8_t serial_map[4] = { 0, SADR-SADR, SCDR-SADR, SDDR-SADR };

/* TODO: stty support for  the r2k ports */
tcflag_t *termios_mask[NUM_DEV_TTY + 1] = {
	0,
	_CSYS,
	_CSYS,
};

void tty_setup(uint8_t minor, uint8_t flags)
{
    minor;
}

/* For the moment */
int tty_carrier(uint8_t minor)
{
    minor;
    return 1;
}

/*
 *	Drain the interrupt queue
 */
void tty_pollirq(void)
{
    uint16_t c;
    while((c = sera_get()) != 0xFF)
            tty_inproc(1, c);
    while((c = serc_get()) != 0xFF)
            tty_inproc(2, c);
    while((c = serd_get()) != 0xFF)
            tty_inproc(3, c);
}

void tty_putc(uint8_t minor, unsigned char c)
{
    out(SADR + serial_map[minor], c);
}

void tty_sleeping(uint8_t minor)
{
    minor;
}

void tty_data_consumed(uint8_t minor)
{
}

ttyready_t tty_writeready(uint8_t minor)
{
    uint8_t r = in(SASR + serial_map[minor]);
    /* FIXME: should go IRQ driven */
    if (r & 4)
        return TTY_READY_SOON;
    else
        return TTY_READY_NOW;
}

/* kernel writes to system console -- never sleep! */
void kputchar(char c)
{
    while(tty_writeready(TTYDEV) != TTY_READY_NOW);
    tty_putc(TTYDEV & 0xFF, c);
    if(c == '\n') {
        while(tty_writeready(TTYDEV) != TTY_READY_NOW);
        tty_putc(TTYDEV & 0xFF, '\r');
    }
}
