#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <tty.h>
#include <devtty.h>

static char tbuf1[TTYSIZ];

/* For now just the Z8 console port */
struct  s_queue  ttyinq[NUM_DEV_TTY+1] = {       /* ttyinq[0] is never used */
    {   NULL,    NULL,    NULL,    0,        0,       0    },
    {   tbuf1,   tbuf1,   tbuf1,   TTYSIZ,   0,   TTYSIZ/2 },
};

/* TODO: stty support */
tcflag_t termios_mask[NUM_DEV_TTY + 1] = {
	0,
	_CSYS
};

/* Low bits are T0, upper bits are divider */
/* The requirement is clock / (128 * divider * prescale) */

#ifdef CONFIG_CLK_14M7
static const uint16_t baudtab[] = {
	0,
	/* Prescale by 12 */
	0x39C0,			/* 50 */
	0x3980,			/* 75 */
	/* Prescale by 8 */
	0x2183,			/* 110 */
	/* Prescale by 6 */
	0x0DAE,			/* 134.5 */
	0x1180,			/* 150 */
	/* Prescale by 3 */
	0x0D80,			/* 300 */
	0x0D40,			/* 600 */
	0x0D20,			/* 1200 */
	0x0D10,			/* 2400 */
	0x0D08,			/* 4800 */
	0x0D04,			/* 9600 */
	0x0D02,			/* 19200 */
	0x0D01,			/* 38400 */
	/* Prescale by 1 */
	0x0502,			/* 57600 */
	0x0501,			/* 115200 */
};
#else
/* Curently set up for 7.3728MHz */
static const uint16_t baudtab[] = {
	0,
	/* Prescale by 6 */
	0x19C0,			/* 50 */
	0x1980,			/* 75 */
	/* Need 523, which alas is prime so go closest */
	/* Prescale by 4 */
	0x19C0,			/* 110 */
	/* Prescale by 3 */
	0x0DAE,			/* 134.5 */
	0x0D80,			/* 150 */
	0x0D40,			/* 300 */
	0x0D20,			/* 600 */
	0x0D10,			/* 1200 */
	0x0D08,			/* 2400 */
	0x0D04,			/* 4800 */
	0x0D02,			/* 9600 */
	0x0D01,			/* 19200 */
	0x00,			/* 38400 */
	/* Prescale by 1 */
	0x0501,			/* 57600 */
	0x00,			/* 115200 */
};
#endif

void tty_setup(uint_fast8_t minor, uint_fast8_t flags)
{
    /* Only some parts do parity and only even so don't bother */
	register struct termios *t = &ttydata[minor].termios;
	register uint16_t baud = t->c_cflag & CBAUD;
	if (baudtab[baud] == 0) {
	    t->c_cflag &= ~CBAUD;
	    t->c_cflag |= B1200;
	    baud = B1200;
        }
        /* Has to be via an asm belper */
        /* TODO set_t0(baudtab[baud]); */
}

/* For the moment */
int tty_carrier(uint_fast8_t minor)
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
    while((c = z8tty_get()) != 0xFFFF)
            tty_inproc(1, c);
}

void tty_putc(uint_fast8_t minor, unsigned c)
{
    z8tty_put(c);
}

void tty_sleeping(uint_fast8_t minor)
{
    minor;
}

void tty_data_consumed(uint_fast8_t minor)
{
}

ttyready_t tty_writeready(uint_fast8_t minor)
{
    if (z8tty_status() & 0x10)
        return TTY_READY_NOW;
    else
        return TTY_READY_SOON;
}

/* kernel writes to system console -- never sleep! */
void kputchar(register unsigned c)
{
    while(tty_writeready(TTYDEV) != TTY_READY_NOW);
    tty_putc(TTYDEV & 0xFF, c);
    if(c == '\n') {
        while(tty_writeready(TTYDEV) != TTY_READY_NOW);
        tty_putc(TTYDEV & 0xFF, '\r');
    }
}
