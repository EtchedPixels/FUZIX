#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <devtty.h>
#include <device.h>
#include <vt.h>
#include <tty.h>

/* Onboard UART */
static volatile uint8_t *cpuio = (volatile uint8_t *)0;

static char tbuf1[TTYSIZ];
PTY_BUFFERS;

struct s_queue ttyinq[NUM_DEV_TTY + 1] = {	/* ttyinq[0] is never used */
	{NULL, NULL, NULL, 0, 0, 0},
	{tbuf1, tbuf1, tbuf1, TTYSIZ, 0, TTYSIZ / 2},
	PTY_QUEUES
};

tcflag_t termios_mask[NUM_DEV_TTY + 1] = {
	0,
	_CSYS	/* TODO */
};

/* Output for the system console (kprintf etc) */
void kputchar(uint8_t c)
{
	if (c == '\n')
		tty_putc(1, '\r');
	tty_putc(1, c);
}

ttyready_t tty_writeready(uint8_t minor)
{
	/* Console is the 6803 onboard port */
	if (cpuio[0x11] & 0x20)
		return TTY_READY_NOW;
	return TTY_READY_SOON;
}

void tty_putc(uint8_t minor, unsigned char c)
{
	while(!(cpuio[0x11] & 0x20));	/* Hack FIXME */
	cpuio[0x13] = c;
}

void tty_setup(uint8_t minor, uint8_t flag)
{
	/* Fudge for now - it is set up by the boot ROM */
}

void tty_sleeping(uint8_t minor)
{
}

/* No carrier signal */
int tty_carrier(uint8_t minor)
{
	return 1;
}

/* No flow control */
void tty_data_consumed(uint8_t minor)
{
}

void tty_poll(void)
{
	if (cpuio[0x11] & 0x80)
		tty_inproc(1, cpuio[0x12]);
}
