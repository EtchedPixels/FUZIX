#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <tty.h>
#include <devtty.h>

static char tbuf1[TTYSIZ];

/* __banked is a hack because right now the compiler isn't generating out0
   as the docs say it does for ez80. Need to rebase sdcc and check if a bug
   is needed */

__sfr __banked __at 0xC0 ttydatap;
__sfr __banked __at 0xC5 ttystat;

struct  s_queue  ttyinq[NUM_DEV_TTY+1] = {       /* ttyinq[0] is never used */
    {   NULL,    NULL,    NULL,    0,        0,       0    },
    {   tbuf1,   tbuf1,   tbuf1,   TTYSIZ,   0,   TTYSIZ/2 },
};

/* We have no actual controls on the virtual ports */
tcflag_t termios_mask[NUM_DEV_TTY + 1] = {
	0,
        _CSYS
};

static uint8_t ttypoll;

/* Write to system console */
void kputchar(char c)
{
    /* handle CRLF */
    if(c=='\n') {
        while(tty_writeready(1) != TTY_READY_NOW);
        tty_putc(1, '\r');
    }
    while(tty_writeready(1) != TTY_READY_NOW);
    tty_putc(1, c);
}

char tty_writeready(uint8_t minor)
{
    used(minor);
    if (ttystat & 0x20)
        return TTY_READY_NOW;
    return TTY_READY_SOON;
}

void tty_putc(uint8_t minor, unsigned char c)
{
    used(minor);
    used(c);

    while (!(ttystat & 0x20));
    ttydatap = c;
}

void tty_sleeping(uint8_t minor)
{
    ttypoll |= 1 << minor;
}

/* Called every timer tick */

static uint8_t tmpReg;

void tty_pollirq(void)
{
    if (ttystat & 1)
        tty_inproc(1, ttydatap);
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
