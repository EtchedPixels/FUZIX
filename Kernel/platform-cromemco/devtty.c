#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <tty.h>
#include <devtty.h>

char tbuf1[TTYSIZ];
char tbuf2[TTYSIZ];
char tbuf3[TTYSIZ];

struct  s_queue  ttyinq[NUM_DEV_TTY+1] = {       /* ttyinq[0] is never used */
    {   NULL,    NULL,    NULL,    0,        0,       0    },
    {   tbuf1,   tbuf1,   tbuf1,   TTYSIZ,   0,   TTYSIZ/2 },
    {   tbuf2,   tbuf2,   tbuf2,   TTYSIZ,   0,   TTYSIZ/2 },
    {   tbuf3,   tbuf3,   tbuf3,   TTYSIZ,   0,   TTYSIZ/2 },
};

static uint8_t ttybase[NUM_DEV_TTY+1] = {
 0, 0, 32, 80
};

tcflag_t uart_mask[4] = {
	_ISYS,
	_OSYS,
	CBAUD|CSTOPB|_CSYS,
	_LSYS,
};

tcflag_t *termios_mask[NUM_DEV_TTY + 1] = {
	NULL,
	uart_mask,
	uart_mask,
	uart_mask
};

static uint8_t ttypoll;

/* Write to system console */
void kputchar(char c)
{
    /* handle CRLF */
    if(c=='\n')
        tty_putc(1, '\r');
    tty_putc(1, c);
}

char tty_writeready(uint8_t minor)
{
    uint8_t r = ttybase[minor];
    r = in(r);
    if (r & 0x80)
        return 1;
    return 0;
}

void tty_putc(uint8_t minor, unsigned char c)
{
    uint8_t r = ttybase[minor];
    out(r + 1, c);
}

void tty_sleeping(uint8_t minor)
{
    ttypoll |= 1 << minor;
}

void tty_data_consumed(uint8_t minor)
{
}

__sfr __at 0x08 timer4;

void tty_irq(uint8_t minor)
{
    uint8_t r = ttybase[minor];
    uint8_t op;
    while((op = in(r + 3)) != 0xFF) {
        switch(op) {
        case 0xE7:
            /* should check in(r) for error bits */
            tty_inproc(minor, in(r + 1 ));
        case 0xEF:
            ttypoll &= ~(1 << minor);
            wakeup(&ttydata[minor]);
        case 0xF7:
            if (minor == 1) {
                timer_interrupt();
                timer4 = 156;
            }
        }
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
    out(r, baudbits[baud] | (t->c_cflag & CSTOPB) ? 0 : 0x80);
    /* Assume we keep to IM1 */
    if (baud <= B9600)
        out(r + 2, 0x8);
    else
        out(r + 2, 0x18);
}

/* For the moment */
int tty_carrier(uint8_t minor)
{
    used(minor);
    return 1;
}
