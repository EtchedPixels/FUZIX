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
__sfr __at 50 tty4stat;
__sfr __at 51 tty4data;

static char tbuf1[TTYSIZ];
static char tbuf2[TTYSIZ];
static char tbuf3[TTYSIZ];
static char tbuf4[TTYSIZ];

struct  s_queue  ttyinq[NUM_DEV_TTY+1] = {       /* ttyinq[0] is never used */
    {   NULL,    NULL,    NULL,    0,        0,       0    },
    {   tbuf1,   tbuf1,   tbuf1,   TTYSIZ,   0,   TTYSIZ/2 },
    {   tbuf2,   tbuf2,   tbuf2,   TTYSIZ,   0,   TTYSIZ/2 },
    {   tbuf3,   tbuf3,   tbuf3,   TTYSIZ,   0,   TTYSIZ/2 },
    {   tbuf4,   tbuf4,   tbuf4,   TTYSIZ,   0,   TTYSIZ/2 }
};

/* We have no actual controls on the virtual ports */
tcflag_t termios_mask[NUM_DEV_TTY + 1] = {
	0,
	_CSYS,
	_CSYS,
	_CSYS,
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
    uint8_t s;

    if (minor == 1)
        return TTY_READY_NOW;
    if (minor == 2)
        s = tty2stat;
    else if (minor == 3)
        s = tty3stat;
    else
        s = tty4stat;
    return s & 2 ? TTY_READY_NOW: TTY_READY_LATER;
}

void tty_putc(uint8_t minor, unsigned char c)
{
    if (minor == 1)
        tty1data = c;
    else if (minor == 2)
        tty2data = c;
    else if (minor == 3)
        tty3data = c;
    else
        tty4data = c;
}

void tty_sleeping(uint8_t minor)
{
    ttypoll |= 1 << minor;
}

/* Called every timer tick */
/* This looks a bit odd but we have the classic emulator problem of the
   serial ports suddenly being empty then getting blasted with data from
   an unrealistic fifo running at what is effectively MHz+ speeds. So we
   gate the number of bytes we allow per clock to simulate a real baud rate
   (in this case 100Hz 80 chars  ~= 9600 baud input) */
void tty_pollirq(void)
{
    unsigned char c; 	/* sdcc bug workaround */
    unsigned char l;

    l = 80;
    while(tty1stat && l--) {
        c = tty1data;
        tty_inproc(1, c);
    }
    l = 80;
    while((tty2stat & 1) &&& l--) {
        c = tty2data;
        tty_inproc(2, c);
    }
    l = 80;
    while((tty3stat & 1) && l--) {
        c = tty3data;
        tty_inproc(3, c);
    }
    l = 80;
    while((tty4stat & 1) && l--) {
        c = tty4data;
        tty_inproc(4, c);
    }
    if ((ttypoll & 4) && (tty2stat & 2)) {
        ttypoll &= ~4;
        wakeup(&ttydata[2]);
    }
    if ((ttypoll & 8) && (tty3stat & 2)) {
        ttypoll &= ~8;
        wakeup(&ttydata[3]);
    }
    if ((ttypoll & 16) && (tty4stat & 2)) {
        ttypoll &= ~16;
        wakeup(&ttydata[4]);
    }
}    

void tty_setup(uint8_t minor, uint8_t flags)
{
    used(minor);
}

/* For the moment */
int tty_carrier(uint8_t minor)
{
    used(minor);
    return 1;
}

void tty_data_consumed(uint8_t minor)
{
}
