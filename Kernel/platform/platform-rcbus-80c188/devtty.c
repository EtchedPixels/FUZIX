#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <devtty.h>
#include <device.h>
#include <tty.h>

static unsigned char tbuf1[TTYSIZ];
static uint8_t sleeping;

tcflag_t termios_mask[NUM_DEV_TTY + 1] = {
	0,
	_CSYS
};

struct s_queue ttyinq[NUM_DEV_TTY + 1] = {	/* ttyinq[0] is never used */
	{NULL, NULL, NULL, 0, 0, 0},
	{tbuf1, tbuf1, tbuf1, TTYSIZ, 0, TTYSIZ / 2},
};

/* Output for the system console (kprintf etc) */
void kputchar(uint_fast8_t c)
{
	while(tty_writeready(1) != TTY_READY_NOW);
	if (c == '\n')
		tty_putc(1, '\r');
	while(tty_writeready(1) != TTY_READY_NOW);
	tty_putc(1, c);
}

ttyready_t tty_writeready(uint8_t minor)
{
	uint8_t c = inb(0xC5);
	return (c & 0x20) ? TTY_READY_NOW : TTY_READY_SOON;
}

void tty_putc(uint8_t minor, unsigned char c)
{
	/* Just the console for now */
	outb(c, 0xC0);
}

void tty_setup(uint_fast8_t minor, uint_fast8_t flags)
{
}

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

/* Currently run off the timer */
void tty_interrupt(void)
{
	uint8_t r = inb(0xC5);
	if (r & 0x01)
		tty_inproc(1, inb(0xC0));
}

void plt_interrupt(void)
{
	timer_interrupt();
	tty_interrupt();
}
