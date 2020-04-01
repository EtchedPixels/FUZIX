#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <devtty.h>
#include <device.h>
#include <vt.h>
#include <tty.h>

/* 16550A at FEC0 */
static volatile uint8_t *uart = (volatile uint8_t *)0xFEC0;

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
	if (uart[5] & 0x20)
	        return TTY_READY_NOW;
	return TTY_READY_SOON;
}

void tty_putc(uint8_t minor, unsigned char c)
{
	while(!(uart[5] & 0x20));
	uart[0] = c;
}

void tty_setup(uint8_t minor, uint8_t flag)
{
}

void tty_sleeping(uint8_t minor)
{
}

/* For the moment */
int tty_carrier(uint8_t minor)
{
	return 1;
}

void tty_data_consumed(uint8_t minor)
{
}

void tty_poll(void)
{
        uint8_t x;
        
        x = uart[5] & 1;
        if (x) {
        	x = uart[0];
		tty_inproc(1, x);
	}
}
                
