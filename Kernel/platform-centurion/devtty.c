#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <devtty.h>
#include <device.h>
#include <vt.h>
#include <tty.h>

/* Quick hack MUX channel support to get us going */

/* Port 1 is the first and at F200 */
#define muxio ((volatile uint8_t *)0xF1FE)

static char tbuf1[TTYSIZ];
static char tbuf2[TTYSIZ];
static char tbuf3[TTYSIZ];
static char tbuf4[TTYSIZ];
PTY_BUFFERS;

struct s_queue ttyinq[NUM_DEV_TTY + 1] = {	/* ttyinq[0] is never used */
	{NULL, NULL, NULL, 0, 0, 0},
	{tbuf1, tbuf1, tbuf1, TTYSIZ, 0, TTYSIZ / 2},
	{tbuf2, tbuf2, tbuf2, TTYSIZ, 0, TTYSIZ / 2},
	{tbuf3, tbuf3, tbuf3, TTYSIZ, 0, TTYSIZ / 2},
	{tbuf4, tbuf4, tbuf4, TTYSIZ, 0, TTYSIZ / 2},
	PTY_QUEUES
};

tcflag_t termios_mask[NUM_DEV_TTY + 1] = {
	0,
	_CSYS	/* TODO */
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
	volatile uint8_t *p = muxio + 2 * minor;
	if (*p & 0x02)
		return TTY_READY_NOW;
	return TTY_READY_SOON;
}

void tty_putc(uint_fast8_t minor, uint_fast8_t c)
{
	volatile uint8_t *p = muxio + 2 * minor;
	while(!(*p & 0x02));
	*++p = c;
}

void tty_setup(uint_fast8_t minor, uint_fast8_t flag)
{
	/* Fudge for now - it is set up by the boot ROM */
}

void tty_sleeping(uint_fast8_t minor)
{
}

/* No carrier signal */
int tty_carrier(uint_fast8_t minor)
{
	return 1;
}

/* No flow control */
void tty_data_consumed(uint_fast8_t minor)
{
}

/* TODO: irqs and other ports */
void tty_poll(void)
{
	if (muxio[0] & 0x01)
		tty_inproc(1, muxio[1]);
}
