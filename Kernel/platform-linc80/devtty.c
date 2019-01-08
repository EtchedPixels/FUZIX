#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <tty.h>
#include <devtty.h>

static char tbuf1[TTYSIZ];
static char tbuf2[TTYSIZ];

static tcflag_t uart_mask[4] = {
	_ISYS,
	_OSYS,
	CSIZE|CSTOPB|CBAUD|PARENB|PARODD|_CSYS,
	_LSYS
};

tcflag_t *termios_mask[NUM_DEV_TTY + 1] = {
	NULL,
	uart_mask,
	uart_mask
};

struct s_queue ttyinq[NUM_DEV_TTY + 1] = {	/* ttyinq[0] is never used */
	{NULL, NULL, NULL, 0, 0, 0},
	{tbuf1, tbuf1, tbuf1, TTYSIZ, 0, TTYSIZ / 2},
	{tbuf2, tbuf2, tbuf2, TTYSIZ, 0, TTYSIZ / 2},
};

uint8_t sio_r[] = {
	0x03, 0xC1,
	0x04, 0xC4,
	0x05, 0xEA
};

static void sio2_setup(uint8_t minor, uint8_t flags)
{
	used(flags);

	struct termios *t = &ttydata[minor].termios;
	uint8_t r;
	/* Set bits per character */
	sio_r[1] = 0x01 | ((t->c_cflag & CSIZE) << 2);
	r = 0xC4;
	if (t->c_cflag & CSTOPB)
		r |= 0x08;
	if (t->c_cflag & PARENB)
		r |= 0x01;
	if (t->c_cflag & PARODD)
		r |= 0x02;
	sio_r[3] = r;
	sio_r[5] = 0x8A | ((t->c_cflag & CSIZE) << 1);
}

void tty_setup(uint8_t minor, uint8_t flags)
{
	sio2_setup(minor, flags);
	sio2_otir(SIO0_BASE + 4 - minor);	/* minor is 1 or 2 */
	/* We need to do CTS/RTS support and baud setting yet */
}

int tty_carrier(uint8_t minor)
{
        uint8_t c;
	if (minor == 2) {
		SIOA_C = 0;
		c = SIOA_C;
	} else {
		SIOB_C = 0;
		c = SIOB_C;
	}
	if (c & 0x8)
		return 1;
	return 0;
}

void tty_pollirq_sio(void)
{
	static uint8_t old_ca, old_cb;
	uint8_t ca, cb;
	uint8_t progress;

	/* Check for an interrupt */
	SIOA_C = 0;
//	if (!(SIOA_C & 2))
//FIXME		return;

	/* FIXME: need to process error/event interrupts as we can get
	   spurious characters or lines on an unused SIO floating */
	do {
		progress = 0;
		SIOA_C = 0;		// read register 0
		ca = SIOA_C;
		/* Input pending */
		if ((ca & 1) && !fullq(&ttyinq[2])) {
			progress = 1;
			tty_inproc(2, SIOA_D);
		}
		/* Break */
		if (ca & 2)
			SIOA_C = 2 << 5;
		/* Output pending */
		if (ca & 4) {
			tty_outproc(2);
			SIOA_C = 5 << 3;	// reg 0 CMD 5 - reset transmit interrupt pending
		}
		/* Carrier changed */
		if ((ca ^ old_ca) & 8) {
			if (ca & 8)
				tty_carrier_raise(2);
			else
				tty_carrier_drop(2);
		}
		SIOB_C = 0;		// read register 0
		cb = SIOB_C;
		if ((cb & 1) && !fullq(&ttyinq[1])) {
			tty_inproc(1, SIOB_D);
			progress = 1;
		}
		if (cb & 4) {
			tty_outproc(1);
			SIOB_C = 5 << 3;	// reg 0 CMD 5 - reset transmit interrupt pending
		}
		if ((cb ^ old_cb) & 8) {
			if (cb & 8)
				tty_carrier_raise(1);
			else
				tty_carrier_drop(1);
		}
	} while(progress);
}

void tty_putc(uint8_t minor, unsigned char c)
{
	if (minor == 2) {
		SIOA_D = c;
	} else if (minor == 1)
		SIOB_D = c;
}

/* We will need this for SIO once we implement flow control signals */
void tty_sleeping(uint8_t minor)
{
	used(minor);
}

/* Be careful here. We need to peek at RR but we must be sure nobody else
   interrupts as we do this. Really we want to switch to irq driven tx ints
   on this platform I think. Need to time it and see

   An asm common level tty driver might be a better idea

   Need to review this we should be ok as the IRQ handler always leaves
   us pointing at RR0 */

ttyready_t tty_writeready(uint8_t minor)
{
	irqflags_t irq;
	uint8_t c;

	irq = di();
	if (minor == 2) {
		SIOA_C = 0;	/* read register 0 */
		c = SIOA_C;
		irqrestore(irq);
		if (c & 0x04)	/* THRE? */
			return TTY_READY_NOW;
		return TTY_READY_SOON;
	} else if (minor == 1) {
		SIOB_C = 0;	/* read register 0 */
		c = SIOB_C;
		irqrestore(irq);
		if (c & 0x04)	/* THRE? */
			return TTY_READY_NOW;
		return TTY_READY_SOON;
	}
	irqrestore(irq);
	return TTY_READY_NOW;
}

void tty_data_consumed(uint8_t minor)
{
	used(minor);
}

/* kernel writes to system console -- never sleep! */
void kputchar(char c)
{
	tty_putc(TTYDEV - 512, c);
	if (c == '\n')
		tty_putc(TTYDEV - 512, '\r');
}
