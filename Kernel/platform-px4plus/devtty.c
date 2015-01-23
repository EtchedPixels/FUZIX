#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <tty.h>
#include <vt.h>
#include <devtty.h>

/* Console is only port we provide, port 2 there but used for floppies */
char tbuf1[TTYSIZ];

struct  s_queue  ttyinq[NUM_DEV_TTY+1] = {       /* ttyinq[0] is never used */
    {   NULL,    NULL,    NULL,    0,        0,       0    },
    {   tbuf1,   tbuf1,   tbuf1,   TTYSIZ,   0,   TTYSIZ/2 },
};

/* Write to system console */
void kputchar(char c)
{
    /* handle CRLF */
    if(c=='\n')
        tty_putc(1, '\r');
    tty_putc(1, c);
}

void tty_putc(uint8_t minor, unsigned char c)
{
    minor;
    vtoutput(&c, 1);
}

void tty_setup(uint8_t minor)
{
    minor;
}

int tty_carrier(uint8_t minor)
{
    minor;
    return 1;
}
