#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <devtty.h>
#include <tty.h>

/*
 *	For now a 16x50 driver for a single port. We actually want to support
 *	both the 16x50 (one or two ports and the 26C92 board that is the
 *	intended companion card (dual serial, timer interrupt and bitbang SPI)
 */

static unsigned char tbuf1[TTYSIZ];

struct s_queue ttyinq[NUM_DEV_TTY + 1] = {	/* ttyinq[0] is never used */
	{NULL, NULL, NULL, 0, 0, 0},
	{tbuf1, tbuf1, tbuf1, TTYSIZ, 0, TTYSIZ / 2},
};

static uint16_t ttyport[] = { 0, 0xC0 };

tcflag_t termios_mask[NUM_DEV_TTY + 1] = {
	0,
	_CSYS|CBAUD|PARENB|PARODD|CSTOPB|CSIZE|CRTSCTS,
};

/* Output for the system console (kprintf etc). Polled. */
void kputchar(uint8_t c)
{
	uint16_t port = ttyport[1];
	if (c == '\n')
		kputchar('\r');
	while(!(in(port + 5) & 0x20));
	out(port, c);
}

ttyready_t tty_writeready(uint8_t minor)
{
	return in(ttyport[minor] + 5) & 0x20 ? TTY_READY_NOW : TTY_READY_SOON;
}

void tty_putc(uint8_t minor, unsigned char c)
{
	out(ttyport[minor], c);
}

/*
 *	16x50 conversion betwen a Bxxxx speed rate (see tty.h) and the values
 *	to stuff into the chip.
 */
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


void tty_setup(uint8_t minor, uint8_t flags)
{
	uint8_t d;
	uint16_t w;
	struct termios *t = &ttydata[minor].termios;
	uint8_t port = ttyport[minor];

	d = 0x80;	/* DLAB (so we can write the speed) */
	d |= (t->c_cflag & CSIZE) >> 4;
	if(t->c_cflag & CSTOPB)
		d |= 0x04;
	if (t->c_cflag & PARENB)
		d |= 0x08;
	if (!(t->c_cflag & PARODD))
		d |= 0x10;
	out(port + 3, d);	/* LCR */
	w = clocks[t->c_cflag & CBAUD];
	out(port, w);		/* Set the DL */
	out(port + 1, w >> 8);
	if (w >> 8)	/* Low speeds interrupt every byte for latency */
		out(port + 2, 0x00);
	else		/* High speeds set our interrupt quite early
			   as our latency is poor, turn on 64 byte if
			   we have a 16C750 */
		out(port + 2, 0x51);
	out(port + 3, d & 0x7F);
	/* FIXME: CTS/RTS support */
	out(port + 4, 0x03); /* DTR RTS */
	out(port + 1, 0x0D); /* We don't use tx ints */
}

/* Not wired on this board */
int tty_carrier(uint8_t minor)
{
	return in(ttyport[minor] + 6) & 0x80;
}

void tty_sleeping(uint8_t minor)
{
}

void tty_data_consumed(uint8_t minor)
{
}

static void tty_interrupt(uint8_t minor)
{
	uint8_t msr;
	uint16_t port = ttyport[minor];
	if (in(port + 5) & 0x01)
		tty_inproc(minor, in(port));
	msr = in(port + 6);
	if (msr & 0x08) {
		if (msr & 0x80)
			tty_carrier_raise(minor);
		else
			tty_carrier_drop(minor);
	}
	/* TOOD: CTS/RTS */
}

void platform_interrupt(void)
{
	tty_interrupt(1);
	/* TODO: a timer source */
//	timer_interrupt();
}

/* Hack for now */
void platform_idle(void)
{
//	tty_interrupt(1);
}
