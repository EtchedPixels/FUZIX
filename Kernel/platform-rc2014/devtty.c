#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <tty.h>
#include <devtty.h>
#include <rc2014.h>
#include "vfd-term.h"
#include "vfd-debug.h"

char tbuf1[TTYSIZ];
char tbuf2[TTYSIZ];

uint8_t ser_type;

struct s_queue ttyinq[NUM_DEV_TTY + 1] = {	/* ttyinq[0] is never used */
	{NULL, NULL, NULL, 0, 0, 0},
	{tbuf1, tbuf1, tbuf1, TTYSIZ, 0, TTYSIZ / 2},
	{tbuf2, tbuf2, tbuf2, TTYSIZ, 0, TTYSIZ / 2},
};

void tty_setup(uint8_t minor)
{
	if (minor == 1) {
	}
}

int tty_carrier(uint8_t minor)
{
//      uint8_t c;
	if (minor == 1) {
//              c = UART0_MSR;
//              return (c & 0x80) ? 1 : 0; /* test DCD */
	}
	return 1;
}

void tty_pollirq_sio(void)
{
	uint8_t ca, cb;

	SIOA_C = 0;		// read register 0
	ca = SIOA_C;
	if (ca & 1) {
		tty_inproc(1, SIOA_D);
	}
	if (ca & 4) {
		tty_outproc(1);
		SIOA_C = 5 << 3;	// reg 0 CMD 5 - reset transmit interrupt pending
	}

	SIOB_C = 0;		// read register 0
	cb = SIOB_C;
	if (cb & 1) {
		tty_inproc(2, SIOB_D);
	}
	if (cb & 4) {
		tty_outproc(2);
		SIOB_C = 5 << 3;	// reg 0 CMD 5 - reset transmit interrupt pending
	}
}

void tty_pollirq_acia(void)
{
	uint8_t ca;

	ca = ACIA_C;
	if (ca & 1) {
		tty_inproc(1, ACIA_D);
	}
	if (ca & 2) {
		tty_outproc(1);
	}
}

void tty_putc(uint8_t minor, unsigned char c)
{
	if (ser_type == 1) {
		if (minor == 1) {
			SIOA_D = c;
#ifdef CONFIG_VFD_TERM
			vfd_term_write(c);
#endif
		} else if (minor == 2)
			SIOB_D = c;
	} else if (minor == 1)
		ACIA_D = c;
	else if (minor = 3) {
		/* FIXME: implement */
	}
}

/* We will need this for SIO once we implement flow control signals */
void tty_sleeping(uint8_t minor)
{
}

ttyready_t tty_writeready(uint8_t minor)
{
	uint8_t c;
	if (ser_type == 1) {
		if (minor == 1) {
			SIOA_C = 0;	/* read register 0 */
			c = SIOA_C;
			if (c & 0x04)	/* THRE? */
				return TTY_READY_NOW;
			return TTY_READY_SOON;
		} else if (minor == 2) {
			SIOB_C = 0;	/* read register 0 */
			c = SIOB_C;
			if (c & 0x04)	/* THRE? */
				return TTY_READY_NOW;
			return TTY_READY_SOON;
		}
	} else if (ser_type == 2 && minor == 1) {
		c = ACIA_C;
		if (c & 0x02)	/* THRE? */
			return TTY_READY_NOW;
		return TTY_READY_SOON;
	}
	return TTY_READY_NOW;
}

void tty_data_consumed(uint8_t minor)
{
}

/* kernel writes to system console -- never sleep! */
void kputchar(char c)
{
	tty_putc(TTYDEV - 512, c);
	if (c == '\n')
		tty_putc(TTYDEV - 512, '\r');
}
