#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <devtty.h>
#include <vt.h>
#include <tty.h>
#include "esp8266_peri.h"
#include "rom.h"

static uint8_t ttybuf[TTYSIZ];

struct s_queue ttyinq[NUM_DEV_TTY+1] = { /* ttyinq[0] is never used */
	{ 0,         0,         0,         0,      0, 0        },
	{ ttybuf,    ttybuf,    ttybuf,    TTYSIZ, 0, TTYSIZ/2 },
};

tcflag_t termios_mask[NUM_DEV_TTY+1] = { 0, _CSYS };

static unsigned int tx_buffer_fill(void)
{
    return (U0S >> USTXC) & 0xff;
}

static unsigned int rx_buffer_fill(void)
{
    return (U0S >> USRXC) & 0xff;
}

static void do_putc(char c)
{
    while (tx_buffer_fill() >= 0x7f)
        ;

    U0F = c;
}

/* Output for the system console (kprintf etc) */
void kputchar(uint_fast8_t c)
{
	if (c == '\n')
		do_putc('\r');
	do_putc(c);
}

void tty_putc(uint_fast8_t minor, uint_fast8_t c)
{
	kputchar(c);
}

void tty_sleeping(uint_fast8_t minor)
{
}

/* FIXME: we should use a smaller target for this than 0x7F so we leave space to avoid blocking on
   echoing */
/* FIXME 2: switch to interrupt and wait burst driven */
ttyready_t tty_writeready(uint_fast8_t minor)
{
    return (tx_buffer_fill() >= 0x7f) ? TTY_READY_SOON : TTY_READY_NOW;
}

/* For the moment */
int tty_carrier(uint_fast8_t minor)
{
    return 1;
}

void tty_data_consumed(uint_fast8_t minor)
{
}

void tty_interrupt(void)
{
    /* FIXME: should check for tty inproc queue fill */
    while (rx_buffer_fill() != 0)
    {
        uint8_t b = U0F;
        tty_inproc(minor(BOOT_TTY), b);
    }
    U0IC = 1 << UITO;
}

void tty_setup(uint_fast8_t minor, uint_fast8_t flags)
{
    irqflags_t irq = di();

    U0C1 = (0x02 << UCTOT)   /* RX timeout threshold */
        | (1 << UCTOE)       /* RX timeout enable */
        ;

    U0IC = 0xffff;           /* clear all pending interrupts */
    U0IE = 1 << UITO;        /* RX timeout enable */

    ets_isr_unmask(1<<ETS_UART_INUM);

    irqrestore(irq);
}

/* vim: sw=4 ts=4 et: */

