#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <devtty.h>
#include <vt.h>
#include <tty.h>

#undef  DEBUG			/* UNdefine to delete debug code sequences */

struct uart16x50 {
	uint8_t data;		/* ls, rx, tx */
	uint8_t msier;		/* ms or ier */
	uint8_t fcr;
	uint8_t lcr;
	uint8_t mcr;
	uint8_t lsr;
	uint8_t msr;
	uint8_t scr;
};

static volatile struct uart16x50 * const uart = (struct uart16x50 *)0xFEC0;

static unsigned char tbuf1[TTYSIZ];
static unsigned char tbuf2[TTYSIZ];

struct s_queue ttyinq[NUM_DEV_TTY + 1] = {	/* ttyinq[0] is never used */
	{NULL, NULL, NULL, 0, 0, 0},
	{tbuf1, tbuf1, tbuf1, TTYSIZ, 0, TTYSIZ / 2},
	{tbuf2, tbuf2, tbuf2, TTYSIZ, 0, TTYSIZ / 2},
};

tcflag_t termios_mask[NUM_DEV_TTY + 1] = {
	0,
	/* 16x50 port 0 */
	/* FIXME CTS/RTS */
	CSIZE|CBAUD|CSTOPB|PARENB|PARODD|_CSYS,
	/* 16x50 port 1 */
	/* FIXME CTS/RTS */
	CSIZE|CBAUD|CSTOPB|PARENB|PARODD|_CSYS
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
	uint8_t c = uart[minor].lsr;
	return (c & 0x20) ? TTY_READY_NOW : TTY_READY_SOON;
}

void tty_putc(uint_fast8_t minor, uint_fast8_t c)
{
	uart[minor].data = c;	/* Data */
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
	struct uart16x50 volatile *u = &uart[minor];
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
	u->lcr = d;
	w = clocks[t->c_cflag & CBAUD];
	u->data = w;		/* ls */
	u->msier = w >> 8;
	u->lcr = d & 0x7F;
	/* FIXME: CTS/RTS support */
	d = 0x03;	/* DTR RTS */
	u->mcr = d;
	u->msier = 0x0D;	/* We don't use tx ints */
}

int tty_carrier(uint_fast8_t minor)
{
	return uart[minor].msr & 0x80;
}

void tty_data_consumed(uint_fast8_t minor)
{
}

void tty_poll(uint8_t minor, struct uart16x50 volatile *u)
{	
	uint8_t msr;

	/* Should be IRQ driven but we might not be so poll anyway if
	   pending. IRQs are off here so this is safe */
	if (u->lsr & 0x01)
		tty_inproc(minor, u->data);
	msr = u->msr;
	/* DCD changed - tell the kernel so it can hangup or open ports */
	if (msr & 0x08) {
		if (msr & 0x80)
			tty_carrier_raise(minor);
		else
			tty_carrier_drop(minor);
	}
	/* TODO: CTS/RTS */
}

void tty_interrupt(void)
{
	tty_poll(1, uart);
	tty_poll(2, uart + 1);
}

void platform_interrupt(void)
{
	tty_interrupt();
	/* TODO */
	timer_interrupt();
	wakeup(&platform_interrupt);
}
