#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <devtty.h>
#include <vt.h>
#include <tty.h>
#include <sci.h>

/* Onboard UART */
static volatile uint8_t *cpuio = (volatile uint8_t *)0xF000;

static char tbuf1[TTYSIZ];
PTY_BUFFERS;

struct s_queue ttyinq[NUM_DEV_TTY + 1] = {	/* ttyinq[0] is never used */
	{NULL, NULL, NULL, 0, 0, 0},
	{tbuf1, tbuf1, tbuf1, TTYSIZ, 0, TTYSIZ / 2},
	PTY_QUEUES
};

tcflag_t termios_mask[NUM_DEV_TTY + 1] = {
	0,
	_CSYS|CBAUD
};

/* Output for the system console (kprintf etc) */
void kputchar(uint8_t c)
{
	if (c == '\n')
		sci_tx_console('\r');
	sci_tx_console(c);
}

ttyready_t tty_writeready(uint8_t minor)
{
	/* 68HC11 onboard port only for now */
	if (sci_tx_space())
		return TTY_READY_NOW;
	return TTY_READY_SOON;
}

void tty_putc(uint8_t minor, unsigned char c)
{
	sci_tx_queue(c);
}

static uint8_t baudtable[16] = {
	0,	/* 0 */
	0xFF,	/* 50 */
	0xFF,	/* 75 */
	0xFF,	/* 110 */
	0x36,	/* 134 (by 13 by 64 = 138 baud) */
	0xFF,	/* 150 */
	0x17,	/* 300 by 3 by 128 */
	0x16,	/* 600 by 3 by 64 */
	0x15,	/* 1200 by 3 by 32 */
	0x14,	/* 2400 by 3 by 16 */
	0x13,	/* 4800 by 3 by 8*/
	0x12,	/* 9600 by 3 by 4*/
	0x11,	/* 19200 by 3 by 2 */
	0x10,	/* 38400 by 3 by 1 */
	0x01,	/* 57600 by 1 by 2 */
	0x00	/* 115200 by 1 by 1 */
};

/* The UART is fairly basic. Any modem lines are just down to GPIO usage. We
   just drive the basic speed control. 7bit and parity are likewise down to
   software so we don't support them */
void tty_setup(uint8_t minor, uint8_t flag)
{
	struct termios *t = &ttydata[minor].termios;
	uint8_t baud = t->c_cflag & CBAUD;
	/* Do 9600 if we can't mamage the one requested */
	if ((baud = baudtable[baud]) == 0xFF) {
		t->c_cflag &= ~CBAUD;
		t->c_cflag |= B9600;
		baud = 0x12;
	}
	cpuio[0x2B] = baud;
	cpuio[0x2D] = 0xAC;	/* tx/rx interrupt on, tx and rx on */
	/* If we turn tx ints on in error the next tx int will turn it back
	   off */
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
	int16_t c = sci_rx_get();
	if (c < 0)
		return;
	tty_inproc(1, (uint8_t)c);
}
