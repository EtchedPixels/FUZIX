#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <devtty.h>
#include <device.h>
#include <vt.h>
#include <tty.h>
#include <graphics.h>
#include <monitor.h>


#undef  DEBUG			/* UNdefine to delete debug code sequences */

static unsigned char tbuf1[TTYSIZ];

struct s_queue ttyinq[NUM_DEV_TTY + 1] = {	/* ttyinq[0] is never used */
	{NULL, NULL, NULL, 0, 0, 0},
	{tbuf1, tbuf1, tbuf1, TTYSIZ, 0, TTYSIZ / 2},
};

tcflag_t termios_mask[NUM_DEV_TTY + 1] = {
	_CSYS,
};

/* tty1 is the screen. Serial can come later */

/* Output for the system console (kprintf etc) */
void kputchar(uint8_t c)
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

void tty_sleeping(uint8_t minor)
{
    used(minor);
}

void tty_setup(uint8_t minor, uint8_t flags)
{
}

int tty_carrier(uint8_t minor)
{
	return 1;
}

void tty_data_consumed(uint8_t minor)
{
}

void plt_interrupt(void)
{
	uint8_t c;
	/* E806 wrapped. Returns value from B in monitor */
	if ((c =  mon_keyboard()) != 0)
		tty_inproc(1, c);
	timer_interrupt();
}

