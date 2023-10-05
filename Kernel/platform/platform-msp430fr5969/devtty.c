#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <devtty.h>
#include <vt.h>
#include <tty.h>
#include "msp430fr5969.h"

__attribute__ ((section (".hidata"))) uint8_t ttybuf[TTYSIZ];
#define ttybuf_hi (queueptr_t)ttybuf

struct s_queue ttyinq[NUM_DEV_TTY+1] = { /* ttyinq[0] is never used */
	{ 0,         0,         0,         0,      0, 0        },
	{ ttybuf_hi, ttybuf_hi, ttybuf_hi, TTYSIZ, 0, TTYSIZ/2 },
};

/* Output for the system console (kprintf etc) */
void kputchar(char c)
{
	if (c == '\n')
		tty_putc(0, '\r');
	tty_putc(0, c);
}

void tty_putc(uint8_t minor, unsigned char c)
{
	while (!(UCA0IFG & UCTXIFG))
		;

	UCA0TXBUF = c;
}

void tty_sleeping(uint8_t minor)
{
}

ttyready_t tty_writeready(uint8_t minor)
{
	return (UCA0IFG & UCTXIFG) ? TTY_READY_NOW : TTY_READY_SOON;
}

void tty_setup(uint8_t minor)
{
	/* Already done on system boot (because we only support one TTY so far). */
}

/* For the moment */
int tty_carrier(uint8_t minor)
{
    return 1;
}

void tty_data_consumed(uint8_t minor)
{
}

void tty_interrupt(void)
{
	tty_inproc(minor(BOOT_TTY), UCA0RXBUF);
}

