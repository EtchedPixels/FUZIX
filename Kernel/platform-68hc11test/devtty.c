#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <devtty.h>
#include <vt.h>
#include <tty.h>
#include <68hc11.h>

static char tbuf1[TTYSIZ];

struct s_queue ttyinq[NUM_DEV_TTY + 1] = {	/* ttyinq[0] is never used */
	{NULL, NULL, NULL, 0, 0, 0},
	{tbuf1, tbuf1, tbuf1, TTYSIZ, 0, TTYSIZ / 2},
};

/* tty1 is the HC11 port B serial */

/* Output for the system console (kprintf etc) */
void kputchar(char c)
{
	if (c == '\n')
		tty_putc(1, '\r');
	tty_putc(1, c);
}

ttyready_t tty_writeready(uint8_t minor)
{
	uint8_t c = *scsr;
	return (c & 0x80) ? TTY_READY_NOW : TTY_READY_SOON;
}

void tty_putc(uint8_t minor, unsigned char c)
{
	*scdr = c;
}

void tty_sleeping(uint8_t minor)
{
	used(minor);
}

/* Assumes 8MHz crystal and SCPR set to 011 */
void tty_setup(uint8_t minor)
{
	uint8_t b = ttydata[1].termios.c_cflag & CBAUD;
	*sccr1 = 0;
	*sccr2 = 0x2C;	/* rx interrupt, rx/tx enabled */
	if (b < B75)
		b = B75;	/* Lowest with fixed divider.. we can do 50
				   but its much mucking about */
	if (b > B9600)
		b = B9600;
	/* Only 8N1 */
	ttydata[1].termios.c_cflag &= ~(CBAUD|PARENB);
	ttydata[1].termios.c_cflag |= b | CS8;
	
	b = B9600 - b;
	
	*baud = b | 0x3;
	minor;
}

/* Carrier etc are done via extra control lines. We don't have any spare! */
int tty_carrier(uint8_t minor)
{
	minor;
	return 1;
}

void platform_interrupt(void)
{
	uint8_t c = *scsr;
	if (c & 0x20) {
		/* Character arrived */
		c = *scdr;	/* This clears the irq as well */
		tty_inproc(1, c);
	}
	timer_interrupt();
}

