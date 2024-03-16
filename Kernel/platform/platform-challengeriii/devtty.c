#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <devtty.h>
#include <vt.h>
#include <tty.h>

/* ACIA at 0xFC00 */
/* Others add per card */
static volatile uint8_t *uart[5] = {
	NULL,
	(volatile uint8_t *) 0xFC00,
	(volatile uint8_t *) 0xCF00,
	(volatile uint8_t *) 0xCF02,
	(volatile uint8_t *) 0xCF04
};

static char tbuf1[TTYSIZ];
static char tbuf2[TTYSIZ];
static char tbuf3[TTYSIZ];
static char tbuf4[TTYSIZ];

struct s_queue ttyinq[NUM_DEV_TTY + 1] = {	/* ttyinq[0] is never used */
	{ NULL, NULL, NULL, 0, 0, 0 },
	{ tbuf1, tbuf1, tbuf1, TTYSIZ, 0, TTYSIZ / 2 },
	{ tbuf2, tbuf2, tbuf2, TTYSIZ, 0, TTYSIZ / 2 },
	{ tbuf3, tbuf3, tbuf3, TTYSIZ, 0, TTYSIZ / 2 },
	{ tbuf4, tbuf4, tbuf4, TTYSIZ, 0, TTYSIZ / 2 },
};

tcflag_t termios_mask[NUM_DEV_TTY + 1] = {
	0,
	/* In theory we have minimal baud control but afaik it was never
	   used that way */
	_CSYS|CSIZE|CSTOPB|PARENB|PARODD,
	_CSYS|CSIZE|CSTOPB|PARENB|PARODD,
	_CSYS|CSIZE|CSTOPB|PARENB|PARODD,
	_CSYS|CSIZE|CSTOPB|PARENB|PARODD,
};

/* Output for the system console (kprintf etc) */
void kputchar(uint_fast8_t c)
{
	if (c == '\t')
		c = ' ';
	if (c == '\n')
		tty_putc(1, '\r');
	tty_putc(1, c);
}

ttyready_t tty_writeready(uint_fast8_t minor)
{
	if (*uart[minor] & 0x02)
		return TTY_READY_NOW;
	return TTY_READY_SOON;
}

void tty_putc(uint_fast8_t minor, uint_fast8_t c)
{
	while (!(*uart[minor] & 0x02));
	uart[minor][1] = c;
}

void tty_setup(uint_fast8_t minor, uint_fast8_t flag)
{
	struct termios *t = &ttydata[minor].termios;
	uint8_t r = t->c_cflag & CSIZE;

	/* No CS5/CS6 CS7 must have parity enabled */
	if (r <= CS7) {
		t->c_cflag &= ~CSIZE;
		t->c_cflag |= CS7|PARENB;
	}
	/* No CS8 parity and 2 stop bits */
	if (r == CS8 && (t->c_cflag & PARENB))
		t->c_cflag &= ~CSTOPB;
	/* There is no obvious logic to this */
	switch(t->c_cflag & (CSIZE|PARENB|PARODD|CSTOPB)) {
	case CS7|PARENB:
		r = 0x8A;
		break;
	case CS7|PARENB|PARODD:
		r = 0x8E;
		break;
	case CS7|PARENB|CSTOPB:
		r = 0x82;
	case CS7|PARENB|PARODD|CSTOPB:
		r = 0x86;
	case CS8|CSTOPB:
		r = 0x92;
		break;
	default:
	case CS8:
		r = 0x96;
		break;
	case CS8|PARENB:
		r = 0x9A;
		break;
	case CS8|PARENB|PARODD:
		r = 0x9E;
		break;
	}
	*uart[minor] = r;
}

void tty_sleeping(uint_fast8_t minor)
{
}

/* For the moment */
int tty_carrier(uint_fast8_t minor)
{
	return 1;
}

void tty_data_consumed(uint_fast8_t minor)
{
}

void tty_poll(void)
{
	uint_fast8_t x;

	if (*uart[1] & 1)
		tty_inproc(1, uart[1][1]);
	if (*uart[2] & 1)
		tty_inproc(2, uart[2][1]);
	if (*uart[3] & 1)
		tty_inproc(3, uart[3][1]);
	if (*uart[4] & 1)
		tty_inproc(4, uart[4][1]);
}
