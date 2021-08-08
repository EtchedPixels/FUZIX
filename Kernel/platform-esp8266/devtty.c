/*
 *	The ESP8266 has two UART devices only one of which can normally be used. The second
 *	shares its input pin with GPIO8 which is used for the external flash. Otherwise it's
 *	a fairly simple device with input and output FIFOs and various optional interrupts. It
 *	can support flow control but we simply don't have enough pins to bother.
 *
 *	We only use the receive interrupt. If we are waiting to transmit then our choices or to wait
 *	or swap another task in which isn't worth it.
 *
 *	TODO: parity.
 */
#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <devtty.h>
#include <vt.h>
#include <tty.h>
#include "esp8266_peri.h"
#include "globals.h"
#include "rom.h"

static uint8_t ttybuf[TTYSIZ];

struct s_queue ttyinq[NUM_DEV_TTY + 1] = {	/* ttyinq[0] is never used */
	{ 0, 0, 0, 0, 0, 0 },
	{ ttybuf, ttybuf, ttybuf, TTYSIZ, 0, TTYSIZ / 2 },
};

tcflag_t termios_mask[NUM_DEV_TTY + 1] = {
	0,
	_CSYS | CBAUD
};

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
	while (tx_buffer_fill() >= 0x7f);

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
	while (rx_buffer_fill() != 0) {
		if (fullq(&ttyinq[1]))
			break;
		uint8_t b = U0F;
		tty_inproc(minor(BOOT_TTY), b);
	}
	U0IC = 1 << UITO;
}

static const uint32_t baudmap[16] = {
	115200,
	0,
	0,
	0,
	0,
	0,
	300,		/* Lowest rate we can do */
	600,
	1200,
	2400,
	4800,
	9600,
	19200,
	38400,
	57600,
	115200
};

void tty_setup(uint_fast8_t minor, uint_fast8_t flags)
{

	irqflags_t irq = di();
	struct termios *t = &ttydata[minor].termios;
	uint16_t baud = baudmap[t->c_cflag & CBAUD];
	if (baud == 0) {
		/* Can't go below 300 baud */
		baud = B300;
		t->c_cflag &= ~CBAUD;
		t->c_cflag |= B300;
	}
	/* Set the speed */
#if 0	
TO DEBUG	uart_div_modify(0, (PERIPHERAL_CLOCK * 1000000) / baud);
#endif
	U0C1 = (0x02 << UCTOT)	/* RX timeout threshold */
	    |(1 << UCTOE)	/* RX timeout enable */
	    ;
	/* FIXME: review IRQ clear */
	U0IC = 0xffff;		/* clear all pending interrupts */
	U0IE = 1 << UITO;	/* RX timeout enable */
	ets_isr_unmask(1 << ETS_UART_INUM);
	irqrestore(irq);
}

/* vim: sw=4 ts=4 et: */
