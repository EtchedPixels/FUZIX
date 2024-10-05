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

tcflag_t termios_mask[NUM_DEV_TTY + 1] = {
	0,
	_CSYS,
	_CSYS,
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

void tty_putc(uint_fast8_t minor, uint_fast8_t c)
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

void tty_sleeping(uint_fast8_t minor)
{
    minor;
}

void tty_data_consumed(uint_fast8_t minor)
{
}

ttyready_t tty_writeready(uint_fast8_t minor)
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
void kputchar(uint_fast8_t c)
{
    tty_putc(TTYDEV & 0xFF, c);
    if(c == '\n')
        tty_putc(TTYDEV & 0xFF, '\r');
}
