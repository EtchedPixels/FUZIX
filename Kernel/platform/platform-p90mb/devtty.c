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
	_CSYS|CBAUD|PARENB|PARODD|CMSPAR,
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
	PUTB(SBUF, tty_add_parity(minor, c));
}

static uint16_t baud_div[] = {
	78,		/* B0: off */
	14976,		/* 50 */
	9984,		/* 75 */
	6807,		/* 110 */
	5567,		/* 134.5 */
	2496,		/* 300 */
	1248,		/* 600 */
	624,		/* 1200 */
	312,		/* 2400 */
	156,		/* 4800 */
	78,		/* 9600 */
	39,		/* 19200 */
	19,		/* 38400 - a bit off  (39410) */
	13,		/* 57600 */
	0		/* 115200 */
};

void tty_setup(uint8_t minor, uint8_t flags)
{
	struct termios *t = &ttydata[minor].termios;
	uint8_t baud = t->c_cflag & CBAUD;

	if (baud == B115200) {
		baud = B57600;
		t->c_cflag &= ~CBAUD;
		t->c_cflag |= baud;
	}
	*(volatile uint16_t *)0x8002052 = -baud_div[baud];
	/* TODO: we can in theory do two stop bits by abusing mode 3 */
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
		tty_inproc_softparity(1,r);
	}	
	/* Ugly - need to move to full IRQ serial I think ? */
	if (r & 0x02)
		PUTB(SCON, GETB(SCON) & 0xFD);
}
