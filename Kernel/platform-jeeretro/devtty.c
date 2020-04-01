#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <tty.h>
#include <devtty.h>

__sfr __at 0 tty1stat;
__sfr __at 1 tty1data;

static char tbuf1[TTYSIZ];

struct  s_queue  ttyinq[NUM_DEV_TTY+1] = {       /* ttyinq[0] is never used */
    {   NULL,    NULL,    NULL,    0,        0,       0    },
    {   tbuf1,   tbuf1,   tbuf1,   TTYSIZ,   0,   TTYSIZ/2 },
};

/* We have no actual controls on the virtual ports */
static tcflag_t port_mask[4] = {
	_ISYS,
	_OSYS,
	_CSYS,
	_LSYS
};

tcflag_t termios_mask[NUM_DEV_TTY + 1] = {
	0,
	_CSYS
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
    minor;
    return TTY_READY_NOW;
}

void tty_putc(uint8_t minor, unsigned char c)
{
    used(minor);
    used(c);

    __asm
        pop hl
        pop de
        push de
        push hl
        ld c,d ; get 2nd arg
        in a, (2)
    __endasm;
}

void tty_sleeping(uint8_t minor)
{
    ttypoll |= 1 << minor;
}

/* Called every timer tick */

void tty_pollirq(void)
{
    if (tty1stat)
        tty_inproc(1, tty1data);
}    

void tty_setup(uint8_t minor, uint8_t flags)
{
    used(minor);
    used(flags);
}

/* For the moment */
int tty_carrier(uint8_t minor)
{
    used(minor);
    return 1;
}

void tty_data_consumed(uint8_t minor)
{
    used(minor);
}
