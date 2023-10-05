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
	_CSYS | CBAUD | CSTOPB | PARENB | PARODD | CSIZE
};

static unsigned int tty_stalled;

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

/*
 *	We have a 128 byte FIFO but we leave a few bytes free for character echo to reduce the likelyhood
 *	of the queue blocking.
 *
 *	TODO: we should see if we can use the transmit interrupt specifically in the case where the
 *	FIFO is full so that we can do waiti instructions and save power until it unblocks.
 */
ttyready_t tty_writeready(uint_fast8_t minor)
{
	return (tx_buffer_fill() >= 0x6f) ? TTY_READY_SOON : TTY_READY_NOW;
}

/* For the moment */
int tty_carrier(uint_fast8_t minor)
{
	return 1;
}

/* Stuff as much from the FIFO into the tty layer as will fit */
static void tty_rx(void)
{
	while (rx_buffer_fill() != 0) {
		if (fullq(&ttyinq[1])) {
			tty_stalled = 1;
			break;
		}
		uint8_t b = U0F;
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
	uint32_t conf = 0;
	irqflags_t irq = di();
	struct termios *t = &ttydata[minor].termios;
	uint16_t baud = baudmap[t->c_cflag & CBAUD];
	if (baud == 0) {
		/* Can't go below 300 baud */
		baud = B300;
		t->c_cflag &= ~CBAUD;
		t->c_cflag |= B300;
	}
	/* Wait for the data to write out */
	if (flags == TCSADRAIN)
		while(tx_buffer_fill());
	/* Parity bits */
	if (t->c_cflag & PARENB) {
		conf |= (1 << UCPAE);
		if (t->c_cflag & PARODD)
			conf |= (1 << UCPA);
	}
	if (t->c_cflag & CSTOPB)
		conf |= ( 3 << UCSBN);
	conf |= ((t->c_cflag & CSIZE) >> 4) << UCBN;
	/* Set the speed */
	U0C0 = conf;
#if 0	
TO DEBUG	uart_div_modify(0, (PERIPHERAL_CLOCK * 1000000) / baud);
#endif
	U0C1 = (0x02 << UCTOT)	/* RX timeout threshold */
	    |(1 << UCTOE)	/* RX timeout enable */
	    ;
	/* FIXME: review IRQ clear */
	U0IC = 0xffff;		/* clear all pending interrupts */
	U0IE = 1 << UITO;	/* RX timeout enable */
	irq_enable(ETS_UART_INUM);
	irqrestore(irq);
}

/* vim: sw=4 ts=4 et: */
