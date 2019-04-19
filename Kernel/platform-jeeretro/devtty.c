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

tcflag_t *termios_mask[NUM_DEV_TTY + 1] = {
	NULL,
	port_mask,
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

static uint8_t tmpReg;

void tty_putc(uint8_t minor, unsigned char c)
{
    minor;

    tmpReg = c;
    __asm
            push bc
            ld a,(_tmpReg);
            ld c,a
	    in a, (2)
            pop bc
    __endasm;
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

    __asm
	    in a, (0)
            ld (_tmpReg), a
    __endasm;

    if (tmpReg == 0)
        return;

    __asm
	    in a, (1)
            ld (_tmpReg), a
    __endasm;

    tty_inproc(1, tmpReg);
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
