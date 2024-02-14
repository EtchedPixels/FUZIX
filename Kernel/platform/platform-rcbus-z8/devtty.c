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

void tty_setup(uint_fast8_t minor, uint_fast8_t flags)
{
    minor;
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
    uint16_t r = z8tty_status();
    /* FIXME: should go IRQ driven */
    if (r & 0x10)
        return TTY_READY_NOW;
    else
        return TTY_READY_SOON;
}

/* kernel writes to system console -- never sleep! */
void kputchar(unsigned c)
{
    while(tty_writeready(TTYDEV) != TTY_READY_NOW);
    tty_putc(TTYDEV & 0xFF, c);
    if(c == '\n') {
        while(tty_writeready(TTYDEV) != TTY_READY_NOW);
        tty_putc(TTYDEV & 0xFF, '\r');
    }
}
