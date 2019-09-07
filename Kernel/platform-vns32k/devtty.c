#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <devtty.h>
#include <tty.h>

/* 68B50 UART */
volatile uint8_t *uart_data = (volatile uint8_t *)0xFFF01;	/* UART data */
volatile uint8_t *uart_status = (volatile uint8_t *)0xFFF00;	/* UART status */

static unsigned char tbuf1[TTYSIZ];

struct s_queue ttyinq[NUM_DEV_TTY + 1] = {	/* ttyinq[0] is never used */
	{NULL, NULL, NULL, 0, 0, 0},
	{tbuf1, tbuf1, tbuf1, TTYSIZ, 0, TTYSIZ / 2},
};

tcflag_t termios_mask[NUM_DEV_TTY + 1] = {
	0,
	_CSYS
};

/* Output for the system console (kprintf etc) */
void kputchar(uint_fast8_t c)
{
	if (c == '\n')
		tty_putc(1, '\r');
	tty_putc(1, c);
}

ttyready_t tty_writeready(uint_fast8_t minor)
{
	uint8_t c = *uart_status;
	return (c & 2) ? TTY_READY_NOW : TTY_READY_SOON; /* TX DATA empty */
}

void tty_putc(uint_fast8_t minor, uint_fast8_t c)
{
	*uart_data = c;	/* Data */
}

void tty_setup(uint_fast8_t minor, uint_fast8_t flags)
{
}

int tty_carrier(uint_fast8_t minor)
{
	return 1;
}

void tty_sleeping(uint_fast8_t minor)
{
}

void tty_data_consumed(uint_fast8_t minor)
{
}

/* Currently run off the timer */
void tty_interrupt(void)
{
	uint8_t r = *uart_status;
	if (r & 1) {
		r = *uart_data;
		tty_inproc(1,r);
	}	
}
