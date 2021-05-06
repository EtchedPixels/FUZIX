#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <devtty.h>
#include <tty.h>


/*
 *	To start with a minimal 16x50 UART driver
 */

static unsigned char tbuf1[TTYSIZ];

struct s_queue ttyinq[NUM_DEV_TTY + 1] = {	/* ttyinq[0] is never used */
	{NULL, NULL, NULL, 0, 0, 0},
	{tbuf1, tbuf1, tbuf1, TTYSIZ, 0, TTYSIZ / 2},
};

static uint8_t sleeping;

tcflag_t termios_mask[NUM_DEV_TTY + 1] = {
	0,
	_CSYS|CBAUD|PARENB|PARODD|CSTOPB|CSIZE|CRTSCTS,
};

/* Output for the system console (kprintf etc). Polled. */
void kputchar(uint8_t c)
{
	if (c == '\n')
		kputchar('\r');
	while(!(in(0xC5) & 0x20));
	out(0xC0, c);
}

ttyready_t tty_writeready(uint8_t minor)
{
	return in(0xC5) & 0x20 ? TTY_READY_NOW : TTY_READY_SOON;
}

void tty_putc(uint8_t minor, unsigned char c)
{
	out(0xC0, c);
}

void tty_setup(uint8_t minor, uint8_t flags)
{
}

/* Not wired on this board */
int tty_carrier(uint8_t minor)
{
	return 1;
}

void tty_sleeping(uint8_t minor)
{
	sleeping |= 1 << minor;
}

void tty_data_consumed(uint8_t minor)
{
}

static void tty_interrupt(void)
{
	if (in(0xC5) & 0x01)
		tty_inproc(1, in(0xC0));
}

void platform_interrupt(void)
{
	tty_interrupt();
	/* TODO: a timer source */
//	timer_interrupt();
}

/* Hack for now */
void platform_idle(void)
{
	tty_interrupt();
}
