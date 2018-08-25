/*
 *	We have a 16x50 UART at 0x68 and maybe a PropIO2 at A8
 *
 *	TODO:
 *	- Hardware flow control
 *	- Support for abuse of 16x50 as interrupt controller
 *	- Support for timer hack
 */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <tty.h>
#include <devtty.h>
#include <propio2.h>

__sfr __at 0x68 uart_tx;
__sfr __at 0x68 uart_rx;
__sfr __at 0x68 uart_ls;
__sfr __at 0x69 uart_ier;
__sfr __at 0x69 uart_ms;
__sfr __at 0x6A uart_fcr;
__sfr __at 0x6B uart_lcr;
__sfr __at 0x6C uart_mcr;
__sfr __at 0x6D uart_lsr;
__sfr __at 0x6E uart_msr;
__sfr __at 0x6F uart_scr;

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

static uint16_t clocks[] = {
	12,		/* Not a real rate */
	2304,
	1536,
	1047,
	857,
	768,
	384,
	192,
	96,
	48,
	24,
	12,
	6,
	3,
	2,
	1
};

void tty_setup(uint8_t minor)
{
	uint8_t d;
	uint16_t w;
	struct termios *t = &ttydata[minor].termios;
	if (ttymap[minor] == 1) {
		/* 16x50. Can actually be configured */
		d = 0x80;	/* DLAB */
		d |= (t->c_cflag & CSIZE) >> 4;
		if(t->c_cflag & CSTOPB)
			d |= 0x04;
		if (t->c_cflag & PARENB)
			d |= 0x08;
		if (!(t->c_cflag & PARODD))
			d |= 0x10;
		uart_lcr = d;
		w = clocks[t->c_cflag & CBAUD];
		uart_ls = w;
		uart_ms = w >> 8;
		uart_lcr = d & 0x7F;
		/* FIXME: CTS/RTS support */
		d = 0x03;	/* DTR RTS */
		uart_mcr = d;
		uart_ier = 0x0D;	/* We don't use tx ints */
	}
}

void tty_sleeping(uint8_t minor)
{
	minor;
}

int tty_carrier(uint8_t minor)
{
        if (ttymap[minor] == 1)
		return uart_msr & 0x80;
	return 1;
}

void tty_data_consumed(uint8_t minor)
{
}

void tty_poll(void)
{	
	/* Should be IRQ driven but we might not be so poll anyway if
	   pending. IRQs are off here so this is safe */
	if (uart_lsr & 0x01)
		tty_inproc(ttymap[1], uart_rx);
	/* If we have a 10MHz clock wired to DSR then do timer interrupts */
	if (timermsr && (uart_msr & 0x40))
		timer_interrupt();
	prop_tty_poll(ttymap[2]);
}
