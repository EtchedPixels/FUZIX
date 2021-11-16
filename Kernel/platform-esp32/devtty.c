#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <devtty.h>
#include <vt.h>
#include <tty.h>
#include "globals.h"
#include "soc/uart_struct.h"
#include "hal/uart_ll.h"

#define UART UART0
static uint8_t ttybuf[TTYSIZ];

struct s_queue ttyinq[NUM_DEV_TTY + 1] = {	/* ttyinq[0] is never used */
	{ 0, 0, 0, 0, 0, 0 },
	{ ttybuf, ttybuf, ttybuf, TTYSIZ, 0, TTYSIZ / 2 },
};

tcflag_t termios_mask[NUM_DEV_TTY + 1] = {
	0,
	_CSYS | CBAUD | CSTOPB | PARENB | PARODD | CSIZE
};

static unsigned int tty_stalled;

static void do_putc(uint8_t c)
{
	while (!UART.int_raw.txfifo_empty)
        ;

    uart_ll_write_txfifo(&UART, &c, 1);
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

ttyready_t tty_writeready(uint_fast8_t minor)
{
	return UART.int_raw.txfifo_empty ? TTY_READY_NOW : TTY_READY_SOON;
}

/* For the moment */
int tty_carrier(uint_fast8_t minor)
{
	return 1;
}

/* Stuff as much from the FIFO into the tty layer as will fit */
static void tty_rx(void)
{
    while (uart_ll_get_rxfifo_len(&UART) != 0) {
		if (fullq(&ttyinq[1])) {
			tty_stalled = 1;
			break;
		}
		uint8_t b;
        uart_ll_read_rxfifo(&UART, &b, 1);
		tty_inproc(minor(BOOT_TTY), b);
	}
}

/* Called when the tty layer has eaten input - try and empty more FIFO */
void tty_data_consumed(uint_fast8_t minor)
{
	irqflags_t irq = di();
	if (tty_stalled == 1) {
		tty_stalled = 0;
		tty_rx();
	}
	irqrestore(irq);
}

/* A tty receive interrupt (we don't enable other types at the moment). Empty the FIFO if we can */
void tty_interrupt(void)
{
	tty_rx();
	//U0IC = 1 << UITO;
}

void tty_setup(uint_fast8_t minor, uint_fast8_t flags)
{
}

/* vim: sw=4 ts=4 et: */
