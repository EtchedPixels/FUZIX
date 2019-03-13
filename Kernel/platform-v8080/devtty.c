#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <tty.h>
#include <devtty.h>

static uint8_t tbuf1[TTYSIZ];
static uint8_t tbuf2[TTYSIZ];

struct  s_queue  ttyinq[NUM_DEV_TTY+1] = {       /* ttyinq[0] is never used */
    {   NULL,    NULL,    NULL,    0,        0,       0    },
    {   tbuf1,   tbuf1,   tbuf1,   TTYSIZ,   0,   TTYSIZ/2 },
    {   tbuf2,   tbuf2,   tbuf2,   TTYSIZ,   0,   TTYSIZ/2 },
};

static tcflag_t console_mask[4] = {
	_ISYS,
	_OSYS,
	_CSYS,
	_LSYS
};

/* TODO: stty support for  the Z180 ports */
tcflag_t *termios_mask[NUM_DEV_TTY + 1] = {
	NULL,
	console_mask,
	console_mask,
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

void tty_putc(uint8_t minor, unsigned char c)
{
    switch(minor){
        case 1:
            ttyout(c);
            break;
        case 2:
            ttyout2(c);
            break;
    }
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
    uint8_t r;
#if 0
    if (minor == 1)
        r = ttyready();
    else
        r = ttyready2();
    if (r)
        return TTY_READY_NOW;
    return TTY_READY_SOON;
#endif
    return TTY_READY_NOW;
}

/* kernel writes to system console -- never sleep! */
void kputchar(char c)
{
    tty_putc(TTYDEV & 0xFF, c);
    if(c == '\n')
        tty_putc(TTYDEV & 0xFF, '\r');
}
