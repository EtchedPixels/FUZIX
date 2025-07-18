/*
 *	TODO: support multiple 16x50 ports
 */
#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <devtty.h>
#include <device.h>
#include <vt.h>
#include <tty.h>

/* 16550A or similar */
static volatile uint8_t *uart = (volatile uint8_t *)0xFEC0;

static char tbuf1[TTYSIZ];
PTY_BUFFERS;

struct s_queue ttyinq[NUM_DEV_TTY + 1] = {	/* ttyinq[0] is never used */
	{NULL, NULL, NULL, 0, 0, 0},
	{tbuf1, tbuf1, tbuf1, TTYSIZ, 0, TTYSIZ / 2},
	PTY_QUEUES
};

tcflag_t termios_mask[NUM_DEV_TTY + 1] = {
	0,
	_CSYS
};

/* Output for the system console (kprintf etc) */
void kputchar(uint8_t c)
{
	if (c == '\n')
		tty_putc(1, '\r');
	tty_putc(1, c);
}

ttyready_t tty_writeready(uint8_t minor)
{
	if (uart[5] & 0x20)
		return TTY_READY_NOW;
	return TTY_READY_SOON;
}

void tty_putc(uint8_t minor, unsigned char c)
{
	*uart = c;
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


void tty_setup(uint8_t minor, uint8_t flag)
{
	uint8_t d;
	uint16_t w;
	struct termios *t = &ttydata[1].termios;

	d = 0x80;	/* DLAB (so we can write the speed) */
	d |= (t->c_cflag & CSIZE) >> 4;
	if(t->c_cflag & CSTOPB)
		d |= 0x04;
	if (t->c_cflag & PARENB)
		d |= 0x08;
	if (!(t->c_cflag & PARODD))
		d |= 0x10;
	uart[3] = d;	/* LCR */
	w = clocks[t->c_cflag & CBAUD];
	*uart = w;		/* Set the DL */
	uart[1] = w >> 8;
	if (w >> 8)	/* Low speeds interrupt every byte for latency */
		uart[2] = 0;
	else		/* High speeds set our interrupt quite early
			   as our latency is poor, turn on 64 byte if
			   we have a 16C750 */
		uart[2] = 0x51;
	uart[3] = d & 0x7F;
	/* FIXME: CTS/RTS support */
	uart[4] = 0x03; /* DTR RTS */
	uart[1] = 0x0D; /* We don't use tx ints */
}

void tty_sleeping(uint8_t minor)
{
}

/* No carrier signal */
int tty_carrier(uint8_t minor)
{
	return uart[6] & 0x80;
}

/* No flow control */
void tty_data_consumed(uint8_t minor)
{
}

void tty_poll(void)
{
	uint8_t msr;
	if (uart[5] & 0x01)
		tty_inproc(1, *uart);
	msr = uart[6];
	if (msr & 0x08) {
		if (msr & 0x80)
			tty_carrier_raise(1);
		else
			tty_carrier_drop(1);
	}
	/* TOOD: CTS/RTS etc */
}
