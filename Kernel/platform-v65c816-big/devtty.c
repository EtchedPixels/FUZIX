#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <devtty.h>
#include <device.h>
#include <vt.h>
#include <tty.h>
#include <io.h>

static char tbuf1[TTYSIZ];
PTY_BUFFERS;

struct s_queue ttyinq[NUM_DEV_TTY + 1] = {	/* ttyinq[0] is never used */
	{NULL, NULL, NULL, 0, 0, 0},
	{tbuf1, tbuf1, tbuf1, TTYSIZ, 0, TTYSIZ / 2},
	PTY_QUEUES
};

tcflag_t termios_mask[NUM_DEV_TTY + 1] = {
	0,
	_CSYS
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
        return TTY_READY_NOW;
}

void tty_putc(uint8_t minor, unsigned char c)
{
	minor;
	poke(0x2000|c);
}

void tty_setup(uint8_t minor, uint8_t flags)
{
	minor;
}

void tty_sleeping(uint8_t minor)
{
	minor;
}

/* For the moment */
int tty_carrier(uint8_t minor)
{
	minor;
	return 1;
}

void tty_data_consumed(uint8_t minor)
{
}

void tty_poll(void)
{
        uint8_t x;
        
        x = peek(0x21) & 1 ;
        if (x) {
        	x = peek(0x20);
		tty_inproc(1, x);
	}
}
                
void plt_interrupt(void)
{
	uint8_t t = peek(0x10);
	tty_poll();
	while(t--) {
		timer_interrupt();
	}
}
