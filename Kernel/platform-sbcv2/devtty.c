/*
 *	We have a 16x50 UART at 0x68 and maybe a PropIO2 at A8
 */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <tty.h>
#include <devtty.h>
#include <propio2.h>

__sfr __at 0x68 uart_tx;
__sfr __at 0x68 uart_rx;
__sfr __at 0x6D uart_lsr;

static char tbuf1[TTYSIZ];
static char tbuf2[TTYSIZ];

/* Updated early in boot to 0,2,1 if PropIO present */
uint8_t ttymap[NUM_DEV_TTY + 1] = {
	0, 1, 2
};

struct s_queue ttyinq[NUM_DEV_TTY + 1] = {	/* ttyinq[0] is never used */
	{NULL, NULL, NULL, 0, 0, 0},
	{tbuf1, tbuf1, tbuf1, TTYSIZ, 0, TTYSIZ / 2},
	{tbuf2, tbuf2, tbuf2, TTYSIZ, 0, TTYSIZ / 2},
};

/* Write to system console */
void kputchar(char c)
{
	/* Need to make console dynamic FIXME */
	if (c == '\n')
		tty_putc(1, '\r');
	tty_putc(1, c);
}

uint8_t tty_writeready(uint8_t minor)
{
	minor;
	/* FIXME: flow control */
	if (ttymap[minor] == 1)
		return uart_lsr & 0x20 ? TTY_READY_NOW : TTY_READY_SOON;
	return prop_tty_writeready();
}

void tty_putc(uint8_t minor, unsigned char c)
{
	minor;
	if (ttymap[minor] == 1)
		uart_tx = c;
	else
		prop_tty_write(c);
}

void tty_setup(uint8_t minor)
{
	minor;
}

void tty_sleeping(uint8_t minor)
{
	minor;
}

int tty_carrier(uint8_t minor)
{
        minor;
	return 1;
}

void tty_data_consumed(uint8_t minor)
{
}

void tty_poll(void)
{	
	/* Should be IRQ driven for this */
	if (uart_lsr & 0x01)
		tty_inproc(ttymap[1], uart_rx);
	prop_tty_poll(ttymap[2]);
}
