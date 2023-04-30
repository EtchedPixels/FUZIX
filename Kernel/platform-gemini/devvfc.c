/*
 *	MAP-80 VFC
 *
 *	A generic 6845 based video controller and a keyboard interface
 *
 *	The vt interface code is in asm partly so we can easily patch
 *	between the Nascom and MAP-80 card but also because it's paged
 *	over system memory so we need to aim straight.
 *
 *	The vt code maps the display and EPROM over F000-F7FF because with
 *	some cards the writes will write through to the underlying memory,
 *	which in that area on a Nascom at least happens to be EPROM.
 */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <tty.h>
#include <vt.h>
#include <devtty.h>
#include <input.h>
#include <devinput.h>
#include <stdarg.h>

char tbuf1[TTYSIZ];
char tbuf2[TTYSIZ];


/* Allow for the fact we might probe/find it at C0 */
uint8_t vfc_iobase = 0xE0;

struct s_queue ttyinq[NUM_DEV_TTY + 1] = {	/* ttyinq[0] is never used */
	{NULL, NULL, NULL, 0, 0, 0},
	{tbuf1, tbuf1, tbuf1, TTYSIZ, 0, TTYSIZ / 2},
	{tbuf2, tbuf2, tbuf2, TTYSIZ, 0, TTYSIZ / 2},
};

/* Write to system console */
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
	vtoutput(&c,1);
}

/* Called every event */
void tty_poll(void)
{
	uint8_t r = in(vfc_iobase + 0xE7);
	if (r !=255)
		tty_inproc(1, r);
}

/* Called on timer events */
void kbd_poll(void)
{
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
	used(minor);
}

void tty_data_consumed(uint8_t minor)
{
	used(minor);
}

