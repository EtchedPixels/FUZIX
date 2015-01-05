#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <tty.h>
#include <devtty.h>

__sfr __at 0 tty1stat;
__sfr __at 1 tty1data;
__sfr __at 40 tty2stat;
__sfr __at 41 tty2data;
__sfr __at 42 tty3stat;
__sfr __at 43 tty3data;

char tbuf1[TTYSIZ];
char tbuf2[TTYSIZ];
char tbuf3[TTYSIZ];

struct  s_queue  ttyinq[NUM_DEV_TTY+1] = {       /* ttyinq[0] is never used */
    {   NULL,    NULL,    NULL,    0,        0,       0    },
    {   tbuf1,   tbuf1,   tbuf1,   TTYSIZ,   0,   TTYSIZ/2 },
    {   tbuf2,   tbuf2,   tbuf2,   TTYSIZ,   0,   TTYSIZ/2 },
    {   tbuf3,   tbuf3,   tbuf3,   TTYSIZ,   0,   TTYSIZ/2 }
};

/* Write to system console */
void kputchar(char c)
{
    /* handle CRLF */
    if(c=='\n')
        tty_putc(1, '\r');
    tty_putc(1, c);
}

bool tty_writeready(uint8_t minor)
{
    uint8_t s;

    if (minor == 1)
        return 1;
    if (minor == 2)
        s = tty2stat;
    else
        s = tty3stat;
    return s & 2;
}

void tty_putc(uint8_t minor, unsigned char c)
{
    if (minor == 1)
        tty1data = c;
    else if (minor == 2)
        tty2data = c;
    else
        tty3data = c;
}

/* Called every timer tick */
void tty_pollirq(void)
{
    unsigned char c; 	/* sdcc bug workaround */
    while(tty1stat) {
        c = tty1data;
        tty_inproc(1, c);
    }
    while(tty2stat & 1) {
        c = tty2data;
        tty_inproc(2, c);
    }
    while(tty3stat & 1) {
        c = tty3data;
        tty_inproc(3, c);
    }
    if (tty2stat & 2)
        wakeup(&ttydata[2]);
    if (tty3stat & 2)
        wakeup(&ttydata[3]);
}    

void tty_setup(uint8_t minor)
{
    minor;
}

/* For the moment */
int tty_carrier(uint8_t minor)
{
    minor;
    return 1;
}
