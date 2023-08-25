#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <devtty.h>
#include <tty.h>

static unsigned char tbuf1[TTYSIZ];

struct s_queue ttyinq[NUM_DEV_TTY + 1] = {	/* ttyinq[0] is never used */
	{NULL, NULL, NULL, 0, 0, 0},
	{tbuf1, tbuf1, tbuf1, TTYSIZ, 0, TTYSIZ / 2},
};

static uint8_t sleeping;

tcflag_t termios_mask[NUM_DEV_TTY + 1] = {
	0,
	_CSYS|CBAUD,
};

static volatile uint8_t *uart_base = (volatile uint8_t *)0x80000000;

#define SBUF	0x2021
#define SCON	0x2023

#define GETB(x)		(uart_base[(x)])
#define PUTB(x,y)	uart_base[(x)] = (y)

/* Output for the system console (kprintf etc). Polled. */
void kputchar(uint8_t c)
{
	/* This can lose receive events .. need to go full IRQ ? */
	if (c == '\n') {
		PUTB(SCON, GETB(SCON) & 0xFD);
		while(!(GETB(SCON) &  2));
		PUTB(SCON, '\r');
	}
	PUTB(SCON, GETB(SCON) & 0xFD);
	while(!(GETB(SCON) &  2));
	PUTB(SBUF, c);
}

ttyready_t tty_writeready(uint8_t minor)
{
	uint8_t c = GETB(SCON);
	return (c & 2) ? TTY_READY_NOW : TTY_READY_SOON; /* TX DATA empty */
}

void tty_putc(uint8_t minor, unsigned char c)
{
	PUTB(SBUF, c);
}

void tty_setup(uint8_t minor, uint8_t flags)
{
	/* Set up baud rate timer */
	*(volatile uint16_t *)0x8002052 = 0xFFF3;
	*(volatile uint8_t *)0x8002055 = 0x34;
	/* FFD9 is 19200, FFB2 is 9600 etc */
}

/* Not wired on this board */
int tty_carrier(uint8_t minor)
{
	return 1;
}

void tty_sleeping(uint8_t minor)
{
	sleeping |= 1 << minor;
}

void tty_data_consumed(uint8_t minor)
{
}

/* Called from the ROM vector helper. We know this is serial */
void tty_interrupt(void)
{
	uint8_t r = GETB(SCON);
	if (r & 0x01) {
		r = GETB(SBUF);
		/* Clear received bit by hand - ugly */
		PUTB(SCON, GETB(SCON) & 0xFE);
		tty_inproc(1,r);
	}	
	/* Ugly - need to move to full IRQ serial I think ? */
	if (r & 0x02)
		PUTB(SCON, GETB(SCON) & 0xFD);
}
