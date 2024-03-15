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
	(volatile uint8_t *) 0xCE02,
	(volatile uint8_t *) 0xCE04,
	(volatile uint8_t *) 0xCE06
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
	_CSYS,			/* TODO */
	_CSYS,			/* TODO */
	_CSYS,			/* TODO */
	_CSYS			/* TODO */
};

/* Output for the system console (kprintf etc) */
void kputchar(uint_fast8_t c)
{
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
		tty_inproc(1, uart[2][1]);
	if (*uart[3] & 1)
		tty_inproc(1, uart[3][1]);
	if (*uart[4] & 1)
		tty_inproc(1, uart[4][1]);
}
