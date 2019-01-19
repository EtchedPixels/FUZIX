#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <tty.h>
#include <devtty.h>
#include <rc2014.h>
#include <scm_monitor.h>

/*
 *	We drive the tty interface via the SCM monitor layer
 *
 *	Right now that costs us RTS/CTS, char size and parity handling
 */

static char tbuf1[TTYSIZ];
static char tbuf2[TTYSIZ];

uint8_t numtty = 1;

static tcflag_t uart0_mask[4] = {
	_ISYS,
	_OSYS,
	CBAUD|_CSYS,
	_LSYS
};

tcflag_t *termios_mask[NUM_DEV_TTY + 1] = {
	NULL,
	uart0_mask,
	uart0_mask
};

struct s_queue ttyinq[NUM_DEV_TTY + 1] = {	/* ttyinq[0] is never used */
	{NULL, NULL, NULL, 0, 0, 0},
	{tbuf1, tbuf1, tbuf1, TTYSIZ, 0, TTYSIZ / 2},
	{tbuf2, tbuf2, tbuf2, TTYSIZ, 0, TTYSIZ / 2},
};

static uint8_t baudmap[] = {
	0, 0, 0, 0, 0, 0,	/* No map for < 300 baud */
	0x0C,	/* 300 */
	0x0B,	/* 600 */
	0x0A,	/* 1200 */
	0x09,	/* 2400 */
	0x08,	/* 4800 */
	0x07,	/* 9600 */
	0x05,	/* 19200 */
	0x04,	/* 38400 */
	0x03,	/* 57600 */
	0x02,	/* 115200 */
};

/* FIXME: TODO set the initial value from the monitor - except it won't tell us */
void tty_setup(uint8_t minor, uint8_t flags)
{
	tcflag_t *tptr = &ttydata[minor].termios.c_cflag;
	/* Safe as we are little endian */
	uint8_t old = *tptr;
	uint8_t baud = old & CBAUD;

	used(flags);

	if (baud < B300) {
		baud = B300;
		old &= ~CBAUD;
		*tptr = old | B300;
	}
	if (!scm_setbaud((((uint16_t)minor) << 8) |baudmap[baud]))
		*tptr = old;
}

int tty_carrier(uint8_t minor)
{
	used(minor);
	return 1;
}

/* We always DI before a ROM call so this is safe */
void tty_pollirq(void)
{
	int16_t c = scm_input(1);
	if (c >= 0)
		tty_inproc(1, c);
	if (numtty > 1) {
		c = scm_input(2);
		if (c >= 0)
			tty_inproc(2, c);
	}
}

/* The SCM monitor doesn't have a check for ready for port n, we have to
   fake stuff up a bit using the supplied 'try and send but might fail' */

static uint16_t ttyq[NUM_DEV_TTY + 1];

void tty_putc(uint8_t minor, unsigned char c)
{
	uint16_t val = ((uint16_t)minor << 8) | c;
	if (scm_output(val))
		ttyq[minor] = val;	/* Minor means it won't be 0 */
}

ttyready_t tty_writeready(uint8_t minor)
{
	/* If nothing is queued we are good */
	if (!ttyq[minor])
		return TTY_READY_NOW;
	/* If we can't write the queued character we are busy */
	if (scm_output(ttyq[minor]))
		return TTY_READY_SOON;
	/* Wipe the queue and report good */
	ttyq[minor] = 0;
	return TTY_READY_NOW;
}

void tty_sleeping(uint8_t minor)
{
	used(minor);
}

void tty_data_consumed(uint8_t minor)
{
	used(minor);
}

/* kernel writes to system console -- never sleep! */
void kputchar(char c)
{
	while(tty_writeready(TTYDEV - 512) != TTY_READY_NOW);
	if (c == '\n')
		tty_putc(TTYDEV - 512, '\r');
	while(tty_writeready(TTYDEV - 512) != TTY_READY_NOW);
	tty_putc(TTYDEV - 512, c);
}
