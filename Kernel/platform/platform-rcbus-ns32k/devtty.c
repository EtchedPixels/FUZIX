#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <devtty.h>
#include <vt.h>
#include <tty.h>

/*
 *	A generic 16x50 UART setup for bootstrap. Will switch to the pluggable
 *	serial driver later
 */

#undef  DEBUG			/* UNdefine to delete debug code sequences */

struct uart16x50 {
	uint8_t data;		/* ls, rx, tx */
	uint8_t pad0;
	uint8_t msier;		/* ms or ier */
	uint8_t pad1;
	uint8_t fcr;
	uint8_t pad2;
	uint8_t lcr;
	uint8_t pad3;
	uint8_t mcr;
	uint8_t pad4;
	uint8_t lsr;
	uint8_t pad5;
	uint8_t msr;
	uint8_t pad6;
	uint8_t scr;
	uint8_t pad7;
};

static volatile struct uart16x50 * const uart = (struct uart16x50 *)0x00F00180;

static unsigned char tbuf1[TTYSIZ];

struct s_queue ttyinq[NUM_DEV_TTY + 1] = {	/* ttyinq[0] is never used */
	{NULL, NULL, NULL, 0, 0, 0},
	{tbuf1, tbuf1, tbuf1, TTYSIZ, 0, TTYSIZ / 2},
};

tcflag_t termios_mask[NUM_DEV_TTY + 1] = {
	0,
	/* 16x50 port 0 */
	CSIZE|CBAUD|CSTOPB|PARENB|PARODD|_CSYS,
};

/* Output for the system console (kprintf etc) */
void kputchar(uint_fast8_t c)
{
	if (c == '\n')
		kputchar('\r');
	while(tty_writeready(1) != TTY_READY_NOW);
	tty_putc(TTYDEV & 0xff, c);
}

ttyready_t tty_writeready(uint_fast8_t minor)
{
	uint8_t c = uart->lsr;
	return (c & 0x20) ? TTY_READY_NOW : TTY_READY_SOON;
}

void tty_putc(uint_fast8_t minor, uint_fast8_t c)
{
	uart->data = c;	/* Data */
}

void tty_sleeping(uint_fast8_t minor)
{
	used(minor);
}

/*
 *	16x50 conversion betwen a Bxxxx speed rate (see tty.h) and the values
 *	to stuff into the chip.
 */

static uint16_t clocks[] = {
	/* TODO: set for actual clocking we use */
	12,
	/* baud 50 divider 9216 result 50 error 0% */
	9216,  /* B50 */
	/* baud 75 divider 6144 result 75 error 0% */
	6144,  /* B75 */
	/* baud 110 divider 4189 result 110 error 0% */
	4189,  /* B110 */
	/* baud 134 divider 3438 result 134 error 0% */
	3438,  /* B134 */
	/* baud 150 divider 3072 result 150 error 0% */
	3072,  /* B150 */
	/* baud 300 divider 1536 result 300 error 0% */
	1536,  /* B300 */
	/* baud 600 divider 768 result 600 error 0% */
	768,  /* B600 */
	/* baud 1200 divider 384 result 1200 error 0% */
	384,  /* B1200 */
	/* baud 2400 divider 192 result 2400 error 0% */
	192,  /* B2400 */
	/* baud 4800 divider 96 result 4800 error 0% */
	96,  /* B4800 */
	/* baud 9600 divider 48 result 9600 error 0% */
	48,  /* B9600 */
	/* baud 19200 divider 24 result 19200 error 0% */
	24,  /* B19200 */
	/* baud 38400 divider 12 result 38400 error 0% */
	12,  /* B38400 */
	/* baud 57600 divider 8 result 57600 error 0% */
	8,  /* B57600 */
	/* baud 115200 divider 4 result 115200 error 0% */
	4,  /* B115200 */
};

/*
 *	This function is called whenever the terminal interface is opened
 *	or the settings changed. It is responsible for making the requested
 *	changes to the port if possible. Strictly speaking it should write
 *	back anything that cannot be implemented to the state it selected.
 */
void tty_setup(uint_fast8_t minor, uint_fast8_t flags)
{
	uint8_t d;
	uint16_t w;
	struct termios *t = &ttydata[minor].termios;
	uint16_t baud = t->c_cflag & CBAUD;

	/* Wait for output to finish */
	if (flags) {
		while(!(uart->lsr & 0x40))
			_sched_yield();
	}
	/* 16x50. Can actually be configured */
	d = 0x80;	/* DLAB (so we can write the speed) */
	d |= (t->c_cflag & CSIZE) >> 4;
	if(t->c_cflag & CSTOPB)
		d |= 0x04;
	if (t->c_cflag & PARENB)
		d |= 0x08;
	if (!(t->c_cflag & PARODD))
		d |= 0x10;
	uart->lcr = d;
	w = clocks[baud];
	if (w == 0) {
		/* Not available */
		w = clocks[B38400];
		t->c_cflag &= ~CBAUD;
		t->c_cflag |= B38400;
	}
	uart->data = w;		/* ls */
	uart->msier = w >> 8;
	uart->lcr = d & 0x7F;
	uart->msier = 0x09;	/* Modem and rx */

	/* FIFO at higher rates, avoid for latency at low */
	if (baud > B1200)
		uart->fcr = 0x87;	/* FIFO 16 byte enable */
	else
		uart->fcr = 0x07;
}

int tty_carrier(uint_fast8_t minor)
{
	return uart[minor - 1].msr & 0x80;
}

void tty_data_consumed(uint_fast8_t minor)
{
}

/* Timer is a hack for now */
static void timer_event(void)
{
	/* External 10Hz generator */
	if (uart->msr & 0x20)
		timer_interrupt();
}

void tty_poll(uint8_t minor, struct uart16x50 volatile *u)
{	
	if (uart->lsr & 0x01)
		tty_inproc(minor, uart->data);
	if (uart->msr & 0x02)
		timer_event();
}

void tty_interrupt(void)
{
	tty_poll(1, uart);
}

void plt_interrupt(void)
{
	tty_interrupt();
}
