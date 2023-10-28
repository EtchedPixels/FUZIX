#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <tty.h>
#include <devtty.h>

#define tty1stat	0
#define tty1data	1
#define tty2stat	40
#define	tty2data	41
#define tty3stat	42
#define	tty3data	43
#define	tty4stat	50
#define	tty4data	51

static char tbuf1[TTYSIZ];
static char tbuf2[TTYSIZ];
static char tbuf3[TTYSIZ];
static char tbuf4[TTYSIZ];

static const uint8_t stat[NUM_DEV_TTY + 1] = {
 0, 0, 40, 42, 50 
};

static const uint8_t data[NUM_DEV_TTY + 1] = {
 0, 1, 41, 43, 51
};

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
void kputchar(uint_fast8_t c)
{
    /* handle CRLF */
    if(c=='\n')
        tty_putc(1, '\r');
    tty_putc(1, c);
}

ttyready_t tty_writeready(uint_fast8_t minor)
{
    if (minor == 1)
        return TTY_READY_NOW;
    return in(stat[minor]) & 2 ? TTY_READY_NOW: TTY_READY_LATER;
}

void tty_putc(uint_fast8_t minor, uint_fast8_t c)
{
    out(data[minor], c);
}

void tty_sleeping(uint_fast8_t minor)
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
    uint_fast8_t l;

    l = 80;
    while(in(tty1stat) && l--)
        tty_inproc(1, in(tty1data));
    l = 80;
    while((in(tty2stat) & 1) && l--)
        tty_inproc(2, in(tty2data));
    l = 80;
    while((in(tty3stat) & 1) && l--)
        tty_inproc(3, in(tty3data));
    l = 80;
    while((in(tty4stat) & 1) && l--)
        tty_inproc(4, in(tty4data));
    if ((ttypoll & 4) && (in(tty2stat) & 2)) {
        ttypoll &= ~4;
        wakeup(&ttydata[2]);
    }
    if ((ttypoll & 8) && (in(tty3stat) & 2)) {
        ttypoll &= ~8;
        wakeup(&ttydata[3]);
    }
    if ((ttypoll & 16) && (in(tty4stat) & 2)) {
        ttypoll &= ~16;
        wakeup(&ttydata[4]);
    }
}    

void tty_setup(uint_fast8_t minor, uint_fast8_t flags)
{
    used(minor);
}

/* For the moment */
int tty_carrier(uint_fast8_t minor)
{
    used(minor);
    return 1;
}

void tty_data_consumed(uint_fast8_t minor)
{
}
