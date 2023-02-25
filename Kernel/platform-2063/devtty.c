#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <tty.h>
#include <devtty.h>
#include <2063.h>

static uint8_t tbuf1[TTYSIZ];
static uint8_t tbuf2[TTYSIZ];

static uint8_t sleeping;

struct s_queue ttyinq[NUM_DEV_TTY + 1] = {	/* ttyinq[0] is never used */
	{NULL, NULL, NULL, 0, 0, 0},
	{tbuf1, tbuf1, tbuf1, TTYSIZ, 0, TTYSIZ / 2},
	{tbuf2, tbuf2, tbuf2, TTYSIZ, 0, TTYSIZ / 2},
};

tcflag_t termios_mask[NUM_DEV_TTY + 1] = {
	0,
	CSIZE|CSTOPB|PARENB|PARODD|_CSYS,
	CSIZE|CBAUD|CSTOPB|PARENB|PARODD|_CSYS,
};

#define SIOC(x)		(SIO0_BASE + 2 + (x) - 1)
#define SIOD(x)		(SIO0_BASE + (x) - 1)
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
	0xC0,	/* 150 */
	0x60,	/* 300 */
	0x30,	/* 600 */
	0x60,	/* 1200 */
	0x30,	/* 2400 */
	0x18,	/* 4800 */
	0x0C,	/* 9600 */
	0x06,	/* 19200 */
	0x03,	/* 38400 */
	0x02,	/* 57600 */
	0x01	/* 115200 */
};

static void sio2_setup(uint_fast8_t minor, uint_fast8_t flags)
{
	struct termios *t = &ttydata[minor].termios;
	uint8_t r;
	uint8_t baud;

	used(flags);

	baud = t->c_cflag & CBAUD;
	if (baud < B150)
		baud = B150;

	sio_r[1] = 0x01 | ((t->c_cflag & CSIZE) << 2);

	/* Pick x64 or x16 */
	r = 0xC4;
	if (baud >= B600)
		r = 0x44;

	if (minor == 1) {
		/* Check SIO/CTC binding */
		CTC_CH1 = 0x55;
		CTC_CH1 = siobaud[baud];
	} else {
		/* Check SIO/CTC binding */
		CTC_CH2 = 0x55;
		CTC_CH2 = siobaud[baud];
	}

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
	sio2_otir(SIOC(minor));
}

int tty_carrier(uint_fast8_t minor)
{
        uint8_t c;
        uint8_t port;
	port = SIOC(minor);
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
			tty_inproc(1, SIOA_D);
		}
		/* Break */
		if (ca & 2)
			SIOA_C = 2 << 5;
		/* Output pending */
		if ((ca & 4) && (sleeping & 4)) {
			tty_outproc(1);
			sleeping &= ~2;
			SIOA_C = 5 << 3;	// reg 0 CMD 5 - reset transmit interrupt pending
		}
		/* Carrier changed */
		if ((ca ^ old_ca) & 8) {
			if (ca & 8)
				tty_carrier_raise(1);
			else
				tty_carrier_drop(1);
		}
		SIOB_C = 0;		// read register 0
		cb = SIOB_C;
		if (cb & 1) {
			tty_inproc(2, SIOB_D);
			progress = 1;
		}
		if ((cb & 4) && (sleeping & 16)) {
			tty_outproc(2);
			sleeping &= ~4;
			SIOB_C = 5 << 3;	// reg 0 CMD 5 - reset transmit interrupt pending
		}
		if ((cb ^ old_cb) & 8) {
			if (cb & 8)
				tty_carrier_raise(2);
			else
				tty_carrier_drop(2);
		}
	} while(progress);
}

void tty_putc(uint_fast8_t minor, uint_fast8_t c)
{
	uint8_t port = SIOD(minor);
	out(port, c);
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

	irq = di();
	port = SIOC(minor);
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
	if (c == '\n')
		kputchar('\r');
	while(tty_writeready(TTYDEV & 0xFF) != TTY_READY_NOW);
	tty_putc(TTYDEV &0xFF, c);
}
