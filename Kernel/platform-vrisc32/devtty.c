#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <devtty.h>
#include <vt.h>
#include <tty.h>

#undef  DEBUG			/* UNdefine to delete debug code sequences */

static unsigned char tbuf1[TTYSIZ];

struct s_queue ttyinq[NUM_DEV_TTY + 1] = {	/* ttyinq[0] is never used */
	{NULL, NULL, NULL, 0, 0, 0},
	{tbuf1, tbuf1, tbuf1, TTYSIZ, 0, TTYSIZ / 2},
};

tcflag_t termios_mask[NUM_DEV_TTY + 1] = {
	0,
	/* 16x50 port 0 */
	CSIZE|CBAUD|CSTOPB|PARENB|PARODD|_CSYS,
};

/* Output for the system console (kprintf etc) */
void kputchar(uint_fast8_t c)
{
	if (c == '\n')
		kputchar('\r');
	while(tty_writeready(1) != TTY_READY_NOW);
	tty_putc(TTYDEV & 0xff, c);
}

ttyready_t tty_writeready(uint_fast8_t minor)
{
	return TTY_READY_NOW;
}

void tty_putc(uint_fast8_t minor, uint_fast8_t c)
{
	*((volatile uint32_t *)0x60000000) = c;	/* Data */
}

void tty_sleeping(uint_fast8_t minor)
{
	used(minor);
}

/*
 *	This function is called whenever the terminal interface is opened
 *	or the settings changed. It is responsible for making the requested
 *	changes to the port if possible. Strictly speaking it should write
 *	back anything that cannot be implemented to the state it selected.
 */
void tty_setup(uint_fast8_t minor, uint_fast8_t flags)
{
}

int tty_carrier(uint_fast8_t minor)
{
	return 1;
}

void tty_data_consumed(uint_fast8_t minor)
{
}

void tty_poll(void)
{	
	if (*((volatile uint32_t *)0x60000004) & 1)
		tty_inproc(1, *((volatile uint32_t *)0x60000000));
}

void tty_interrupt(void)
{
	tty_poll();
}

void plt_interrupt(void)
{
	tty_interrupt();
}
