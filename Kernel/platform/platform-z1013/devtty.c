#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <tty.h>
#include <vt.h>
#include <devtty.h>

static uint8_t tbuf1[TTYSIZ];

static uint8_t sleeping;

uint8_t vtattr_cap = 0;		/* TODO: colour */
struct s_queue ttyinq[NUM_DEV_TTY + 1] = {	/* ttyinq[0] is never used */
	{NULL, NULL, NULL, 0, 0, 0},
	{tbuf1, tbuf1, tbuf1, TTYSIZ, 0, TTYSIZ / 2},
};

tcflag_t termios_mask[NUM_DEV_TTY + 1] = {
	0,
	_CSYS,
};

void tty_setup(uint_fast8_t minor, uint_fast8_t flags)
{
}

int tty_carrier(uint_fast8_t minor)
{
	return 1;
}

void tty_putc(uint_fast8_t minor, uint_fast8_t c)
{
	uint8_t ch = c;
	vtoutput(&ch, 1);
}

void tty_sleeping(uint_fast8_t minor)
{
	sleeping |= (1 << minor);
}

ttyready_t tty_writeready(uint_fast8_t minor)
{
	return TTY_READY_NOW;
}

void tty_data_consumed(uint_fast8_t minor)
{
	used(minor);
}

/* kernel writes to system console -- never sleep! */
void kputchar(uint_fast8_t c)
{
	if (c == '\n')
		tty_putc(TTYDEV - 512, '\r');
	tty_putc(TTYDEV - 512, c);
}
