#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <tty.h>
#include <devtty.h>
#include <z180.h>
#include <n8vem.h>

static uint8_t tbuf1[TTYSIZ];
static uint8_t tbuf2[TTYSIZ];

#ifdef CONFIG_PROPIO2
static uint8_t tbufp[TTYSIZ];
#endif

struct  s_queue  ttyinq[NUM_DEV_TTY+1] = {       /* ttyinq[0] is never used */
    {   NULL,    NULL,    NULL,    0,        0,       0    },
    {   tbuf1,   tbuf1,   tbuf1,   TTYSIZ,   0,   TTYSIZ/2 },
    {   tbuf2,   tbuf2,   tbuf2,   TTYSIZ,   0,   TTYSIZ/2 },
#ifdef CONFIG_PROPIO2
    {   tbufp,   tbufp,   tbufp,   TTYSIZ,   0,   TTYSIZ/2 },
#endif
};

/* TODO: stty support for  the Z180 ports */
tcflag_t termios_mask[NUM_DEV_TTY + 1] = {
	0,
	_CSYS,
	_CSYS,
#ifdef CONFIG_PROPIO2
	_CSYS
#endif
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

void tty_pollirq_asci0(void)
{
    while(ASCI_STAT0 & 0x80)
        tty_inproc(1, ASCI_RDR0);
    if (ASCI_STAT0 & 0x70)
        ASCI_CNTLA0 &= ~0x08;
}

void tty_pollirq_asci1(void)
{
    while(ASCI_STAT1 & 0x80)
        tty_inproc(2, ASCI_RDR1);
    if (ASCI_STAT1 & 0x70)
        ASCI_CNTLA1 &= ~0x08;
}

#ifdef CONFIG_PROPIO2
void tty_poll_propio2(void)
{
    while(PROPIO2_STAT & 0x20)
        tty_inproc(3, PROPIO2_TERM);
}
#endif

void tty_putc(uint_fast8_t minor, uint_fast8_t c)
{
    switch(minor){
        case 1:
            while(!(ASCI_STAT0 & 2));
            ASCI_TDR0 = c;
            break;
        case 2:
            while(!(ASCI_STAT1 & 2));
            ASCI_TDR1 = c;
            break;
#ifdef CONFIG_PROPIO2
        case 3:
            while(!(PROPIO2_STAT & 0x10));
            PROPIO2_TERM = c;
            break;
#endif
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
    minor;
    return TTY_READY_NOW;
}

/* kernel writes to system console -- never sleep! */
void kputchar(uint_fast8_t c)
{
    tty_putc(TTYDEV & 0xFF, c);
    if(c == '\n')
        tty_putc(TTYDEV & 0xFF, '\r');
}
