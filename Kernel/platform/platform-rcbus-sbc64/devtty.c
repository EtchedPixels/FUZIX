#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <tty.h>
#include <devtty.h>
#include <rcbus.h>

__sfr __at 0xf8 cpld_status;
__sfr __at 0xf9 cpld_data;

static uint8_t tbuf1[TTYSIZ];
static uint8_t tbuf2[TTYSIZ];
static uint8_t tbuf3[TTYSIZ];
static uint8_t tbuf4[TTYSIZ];
static uint8_t tbuf5[TTYSIZ];

static uint8_t sleeping;

struct s_queue ttyinq[NUM_DEV_TTY + 1] = {	/* ttyinq[0] is never used */
	{NULL, NULL, NULL, 0, 0, 0},
	{tbuf1, tbuf1, tbuf1, TTYSIZ, 0, TTYSIZ / 2},
	{tbuf2, tbuf2, tbuf2, TTYSIZ, 0, TTYSIZ / 2},
	{tbuf3, tbuf3, tbuf3, TTYSIZ, 0, TTYSIZ / 2},
	{tbuf4, tbuf2, tbuf2, TTYSIZ, 0, TTYSIZ / 2},
	{tbuf5, tbuf3, tbuf3, TTYSIZ, 0, TTYSIZ / 2},
};

tcflag_t termios_mask[NUM_DEV_TTY + 1] = {
	0,
	_CSYS,
	/* FIXME CTS/RTS */
	CSIZE|CSTOPB|PARENB|PARODD|_CSYS,
	CSIZE|CBAUD|CSTOPB|PARENB|PARODD|_CSYS,
	CSIZE|CSTOPB|PARENB|PARODD|_CSYS,
	CSIZE|CBAUD|CSTOPB|PARENB|PARODD|_CSYS,
};

uint8_t sio_r[] = {
	0x03, 0xC1,
	0x04, 0xC4,
	0x05, 0xEA
};

static uint16_t siobaud[] = {
	0xC0,	/* 0 */
	0,	/* 50 */
	0,	/* 75 */
	0,	/* 110 */
	0,	/* 134 */
	0,	/* 150 */
	0xC0,	/* 300 */
	0x60,	/* 600 */
	0xC0,	/* 1200 */
	0x60,	/* 2400 */
	0x30,	/* 4800 */
	0x18,	/* 9600 */
	0x0C,	/* 19200 */
	0x06,	/* 38400 */
	0x04,	/* 57600 */
	0x02	/* 115200 */
};

static void sio2_setup(uint_fast8_t minor, uint_fast8_t flags)
{
	struct termios *t = &ttydata[minor].termios;
	uint8_t r;
	uint8_t baud;

	used(flags);

	baud = t->c_cflag & CBAUD;
	if (baud < B300)
		baud = B300;

	/* Set bits per character */
	sio_r[1] = 0x01 | ((t->c_cflag & CSIZE) << 2);

	r = 0xC4;
	if (ctc_present && minor == 3) {
		CTC_CH1 = 0x55;
		CTC_CH1 = siobaud[baud];
		if (baud > B600)	/* Use x16 clock and CTC divider */
			r = 0x44;
	} else
		baud = B115200;

	t->c_cflag &= ~CBAUD;
	t->c_cflag |= baud;

	if (t->c_cflag & CSTOPB)
		r |= 0x08;
	if (t->c_cflag & PARENB)
		r |= 0x01;
	if (t->c_cflag & PARODD)
		r |= 0x02;
	sio_r[3] = r;
	sio_r[5] = 0x8A | ((t->c_cflag & CSIZE) << 1);
}

void tty_setup(uint_fast8_t minor, uint_fast8_t flags)
{
	if (minor == 1)
		return;
	sio2_setup(minor, flags);
	sio2_otir(SIO0_BASE + 2 * (minor - 2));
	/* We need to do CTS/RTS support on channel 2 yet */
}

void tty_resume(void)
{
	tty_setup(1, 0);
	if (sio_present) {
		tty_setup(2, 0);
		tty_setup(3, 0);
	}
	if (sio1_present) {
		tty_setup(4, 0);
		tty_setup(5, 0);
	}
}

int tty_carrier(uint_fast8_t minor)
{
        uint8_t c;
        uint8_t port;
        if (minor == 1)
        	return 1;
	port = SIO0_BASE + 2 * (minor - 2);
	out(port, 0);
	c = in(port);
	if (c & 0x08)
		return 1;
	return 0;
}

void tty_pollirq_sio0(void)
{
	static uint8_t old_ca, old_cb;
	uint8_t ca, cb;
	uint8_t progress;

	/* Check for an interrupt */
	SIOA_C = 0;
	if (!(SIOA_C & 2))
		return;

	/* FIXME: need to process error/event interrupts as we can get
	   spurious characters or lines on an unused SIO floating */
	do {
		progress = 0;
		SIOA_C = 0;		// read register 0
		ca = SIOA_C;
		/* Input pending */
		if (ca & 1) {
			progress = 1;
			tty_inproc(2, SIOA_D);
		}
		/* Break */
		if (ca & 2)
			SIOA_C = 2 << 5;
		/* Output pending */
		if ((ca & 4) && (sleeping & 4)) {
			tty_outproc(2);
			sleeping &= ~4;
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
		if (cb & 1) {
			tty_inproc(3, SIOB_D);
			progress = 1;
		}
		if ((cb & 4) && (sleeping & 16)) {
			tty_outproc(3);
			sleeping &= ~16;
			SIOB_C = 5 << 3;	// reg 0 CMD 5 - reset transmit interrupt pending
		}
		if ((cb ^ old_cb) & 8) {
			if (cb & 8)
				tty_carrier_raise(3);
			else
				tty_carrier_drop(3);
		}
	} while(progress);
}

void tty_pollirq_sio1(void)
{
	static uint8_t old_ca, old_cb;
	uint8_t ca, cb;
	uint8_t progress;

	/* Check for an interrupt */
	SIOC_C = 0;
	if (!(SIOC_C & 2))
		return;

	/* FIXME: need to process error/event interrupts as we can get
	   spurious characters or lines on an unused SIO floating */
	do {
		progress = 0;
		SIOC_C = 0;		// read register 0
		ca = SIOC_C;
		/* Input pending */
		if (ca & 1) {
			progress = 1;
			tty_inproc(4, SIOC_D);
		}
		/* Break */
		if (ca & 2)
			SIOC_C = 2 << 5;
		/* Output pending */
		if ((ca & 4) && (sleeping & 16)) {
			tty_outproc(4);
			sleeping &= ~16;
			SIOC_C = 5 << 3;	// reg 0 CMD 5 - reset transmit interrupt pending
		}
		/* Carrier changed */
		if ((ca ^ old_ca) & 8) {
			if (ca & 8)
				tty_carrier_raise(4);
			else
				tty_carrier_drop(4);
		}
		SIOD_C = 0;		// read register 0
		cb = SIOD_C;
		if (cb & 1) {
			tty_inproc(5, SIOD_D);
			progress = 1;
		}
		if ((cb & 4) && (sleeping & 32)) {
			tty_outproc(5);
			sleeping &= ~32;
			SIOD_C = 5 << 3;	// reg 0 CMD 5 - reset transmit interrupt pending
		}
		if ((cb ^ old_cb) & 8) {
			if (cb & 8)
				tty_carrier_raise(5);
			else
				tty_carrier_drop(5);
		}
	} while(progress);
}

void tty_poll_cpld(void)
{
	uint8_t ca;

	ca = cpld_status;
	if (ca & 1)
		tty_inproc(1, cpld_data);
}

void tty_putc(uint_fast8_t minor, uint_fast8_t c)
{
	if (minor == 1) {
		irqflags_t irq = di();
		cpld_bitbang(c);
		irqrestore(irq);
	}
	else {
		uint8_t port = SIO0_BASE + 1 + 2 * (minor - 2);
		out(port, c);
	}
}

/* We will need this for SIO once we implement flow control signals */
void tty_sleeping(uint_fast8_t minor)
{
	sleeping |= (1 << minor);
}

/* Be careful here. We need to peek at RR but we must be sure nobody else
   interrupts as we do this. Really we want to switch to irq driven tx ints
   on this platform I think. Need to time it and see

   An asm common level tty driver might be a better idea

   Need to review this we should be ok as the IRQ handler always leaves
   us pointing at RR0 */
ttyready_t tty_writeready(uint_fast8_t minor)
{
	irqflags_t irq;
	uint8_t c;
	uint8_t port;

	/* Bitbang port, if the CPU is ready the port is ready */
	if (minor == 1)
		return TTY_READY_NOW;

	irq = di();
	port = SIO0_BASE+ 2 * (minor - 2);
	out(port, 0);
	c = in(port);
	irqrestore(irq);

	if (c & 0x04)	/* THRE? */
		return TTY_READY_NOW;
	return TTY_READY_SOON;
}

void tty_data_consumed(uint_fast8_t minor)
{
	used(minor);
}

/* kernel writes to system console -- never sleep! */
void kputchar(uint_fast8_t c)
{
	/* Always use the bitbang port - no need for write waits therefore */
	if (c == '\n')
		tty_putc(TTYDEV - 512, '\r');
	tty_putc(TTYDEV - 512, c);
}

int rctty_open(uint_fast8_t minor, uint16_t flag)
{
	if ((minor == 2 || minor == 3) && !sio_present) {
		udata.u_error = ENODEV;
		return -1;
	}
	if ((minor == 4 || minor == 5) && !sio1_present) {
		udata.u_error = ENODEV;
		return -1;
	}
	return tty_open(minor, flag);
}
