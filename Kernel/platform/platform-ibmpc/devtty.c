/*
 *	Just to get us going assume we have a console and keyboard and leave
 *	the PC serial ports alone as we can't sanely drive them via BIOS.
 *	We have other stuff to bring up before working on those tricky bits.
 */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <devtty.h>
#include <device.h>
#include <tty.h>
#include <vt.h>

static unsigned char tbuf1[TTYSIZ];

struct s_queue ttyinq[NUM_DEV_TTY + 1] = {	/* ttyinq[0] is never used */
	{NULL, NULL, NULL, 0, 0, 0},
	{tbuf1, tbuf1, tbuf1, TTYSIZ, 0, TTYSIZ / 2},
};

/* Output for the system console (kprintf etc) */
void kputchar(char c)
{
	if (c == '\n')
		tty_putc(1, '\r');
	tty_putc(1, c);
}

ttyready_t tty_writeready(uint8_t minor)
{
	return TTY_READY_NOW;
}

void tty_putc(uint8_t minor, unsigned char c)
{
	vtoutput(&c, 1);
}

void tty_setup(uint8_t minor)
{
}

int tty_carrier(uint8_t minor)
{
	return 1;
}

void tty_sleeping(uint8_t minor)
{
}

void tty_data_consumed(uint8_t minor)
{
}
