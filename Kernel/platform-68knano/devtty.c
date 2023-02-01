#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <devtty.h>
#include <vt.h>
#include <tty.h>

/*
 *	A generic 16x50 UART setup. The only oddity in this case is that the
 *	various moden and control signals are brought out on an extension
 *	and intended to drive an SPI RTC although not in our case.
 *
 *	MISO is wired to \DSR
 *	MOSI is wired to \DTR
 *	CLK is wired to \OUT1
 *	nSS is wired to \OUT2
 *
 *	The original board uses \RI for a button, \DSR for SQW
 *
 *	Until we add the SD card just deal with the tick
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

static volatile struct uart16x50 * const uart = (struct uart16x50 *)0xA00000;
static volatile uint8_t * const ptm = (uint8_t *)0xFE60;

static unsigned char tbuf1[TTYSIZ];

struct s_queue ttyinq[NUM_DEV_TTY + 1] = {	/* ttyinq[0] is never used */
	{NULL, NULL, NULL, 0, 0, 0},
	{tbuf1, tbuf1, tbuf1, TTYSIZ, 0, TTYSIZ / 2},
};

tcflag_t termios_mask[NUM_DEV_TTY + 1] = {
	0,
	/* 16x50 port 0 */
	/* FIXME CTS/RTS */
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
 *
 *	Running a 7.378MHz clock so we can get the baud rates nicely. If
 *	you use other rates beware - the macro doesn't consider rounding
 *	errors and just assumes it all works. In particular the 12MHz option
 *	makes a lot of the rates way off or inaccessible.
 */

#define BAUD(x)		(7372800 / ((x) * 16))

static uint16_t clocks[] = {
	BAUD(9600),	/* Not a real rate */
	BAUD(50),
	BAUD(75),
	BAUD(110),
	
	BAUD(134),
	BAUD(150),
	BAUD(300),
	BAUD(600),
	
	BAUD(1200),
	BAUD(2400),
	BAUD(4800),
	BAUD(9600),

	BAUD(19200),
	BAUD(38400),
	BAUD(57600),
	BAUD(115200)
};

/*
 *	This function is called whenever the terminal interface is opened
 *	or the settings changed. It is responsible for making the requested
 *	changes to the port if possible. Strictly speaking it should write
 *	back anything that cannot be implemented to the state it selected.
 *
 *	That needs tidying up in many platforms and we also need a proper way
 *	to say 'this port is fixed config' before making it so.
 */
void tty_setup(uint_fast8_t minor, uint_fast8_t flags)
{
	uint8_t d;
	uint16_t w;
	struct termios *t = &ttydata[minor].termios;
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
	w = clocks[t->c_cflag & CBAUD];
	uart->data = w;		/* ls */
	uart->msier = w >> 8;
	uart->lcr = d & 0x7F;
	uart->msier = 0x09;	/* Modem and rx */

	/* TODO: FIFO on for higher baud rates */
}

int tty_carrier(uint_fast8_t minor)
{
	return uart[minor - 1].msr & 0x80;
}

void tty_data_consumed(uint_fast8_t minor)
{
}

void tty_poll(uint8_t minor, struct uart16x50 volatile *u)
{	
	uint8_t msr;

	/* Should be IRQ driven but we might not be so poll anyway if
	   pending. IRQs are off here so this is safe */
	if (uart->lsr & 0x01)
		tty_inproc(minor, uart->data);
	msr = uart->msr;
	
	/* DCD changed - tell the kernel so it can hangup or open ports */
	if (msr & 0x08) {
		if (msr & 0x80)
			tty_carrier_raise(minor);
		else
			tty_carrier_drop(minor);
	}
	if (msr & 0x04) {
		if (msr & 0x20)
			timer_interrupt();
	}
}

void tty_interrupt(void)
{
	tty_poll(1, uart);
}

void plt_interrupt(void)
{
	tty_interrupt();
	wakeup(&plt_interrupt);
}
