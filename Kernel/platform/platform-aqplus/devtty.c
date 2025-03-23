#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <tty.h>
#include <vt.h>
#include <devtty.h>
#include <iop.h>

static char tbuf1[TTYSIZ];

static uint8_t sleeping;

struct s_queue ttyinq[NUM_DEV_TTY + 1] = {	/* ttyinq[0] is never used */
	{NULL, NULL, NULL, 0, 0, 0},
	{tbuf1, tbuf1, tbuf1, TTYSIZ, 0, TTYSIZ / 2},
};

tcflag_t termios_mask[NUM_DEV_TTY + 1] = {
	0,
	_CSYS,
};

uint8_t vtattr_cap = 0;

void tty_setup(uint_fast8_t minor, uint_fast8_t flags)
{
}

int tty_carrier(uint_fast8_t minor)
{
	return 1;
}

void tty_putc(uint_fast8_t minor, uint_fast8_t c)
{
	unsigned char cc = c;
	vtoutput(&cc, 1);
}

void tty_sleeping(uint_fast8_t minor)
{
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
		kputchar('\r');
	kputchar(c);
}

/* TODO: switch to scancode mode */
void tty_poll(void)
{
	uint_fast8_t c = in(0xFA);
	if (c)
		vt_inproc(1,c);
}

void do_beep(void)
{
}
