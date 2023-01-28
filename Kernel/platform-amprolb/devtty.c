#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <tty.h>
#include <devtty.h>
#include <lb.h>

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
	/* FIXME CTS/RTS */
	CSIZE|CSTOPB|PARENB|PARODD|_CSYS,
	CSIZE|CBAUD|CSTOPB|PARENB|PARODD|_CSYS,
};

uint8_t dart_r[] = {
	0x03, 0xC1,
	0x04, 0xC4,
	0x05, 0xEA
};

#define TX16	0x0400
#define X16	0x4400
#define X32	0x8400

static uint16_t dartbaud[] = {
	0xC0,		/* 0 */
	0,		/* 50 */
	0,		/* 75 */
	142|TX16,	/* 110 */
	115|TX16,	/* 134 */
	71|TX16,	/* 150 */
	208|X32,	/* 300 */
	208|X16,	/* 600 */
	104|X16,	/* 1200 */
	52|X16,		/* 2400 */
	26|X16,		/* 4800 */
	13|X16,		/* 9600 */
	/* Channel A has special clocking for 19200,38400 */
	32|X16,		/* 19200 */
	16|X16,		/* 38400 */
	0,		/* 57600 */
	0		/* 115200 */
};

static void dart_setup(uint_fast8_t minor, uint_fast8_t flags)
{
	struct termios *t = &ttydata[minor].termios;
	uint8_t r;
	uint8_t ch = 0x47;	/* Counter mode */
	uint8_t baud;
	uint8_t dtr = 0x80;	/* DTR high */

	used(flags);

	baud = t->c_cflag & CBAUD;
	if (baud < B110)
		baud = B110;
	if (baud > 38400)
		baud = B38400;
	if (minor !=1 && baud > B9600)
		baud = B9600;
	
	/* Set bits per character */
	dart_r[1] = 0x01 | ((t->c_cflag & CSIZE) << 2);

	r = dartbaud[baud] >> 8;
	
	if (baud < B300)
		ch = 0x07;

	/* Channel A can also run off a different clock for > 9600 baud
	   by setting DTRA low */
	if (minor == 1) {
		if (baud > B9600) {
			/* Stop the CTC */
			CTC_CH0 = 0x03;
			/* DTRA low */
			dtr = 0x00;
		} else {		
			/* DTRA high */
			CTC_CH0 = ch;
			CTC_CH0 = dartbaud[baud];
		}
	} else {
		CTC_CH1 = ch;
		CTC_CH1 = dartbaud[baud];
	}

	t->c_cflag &= ~CBAUD;
	t->c_cflag |= baud;

	if (t->c_cflag & CSTOPB)
		r |= 0x08;
	if (t->c_cflag & PARENB)
		r |= 0x01;
	if (t->c_cflag & PARODD)
		r |= 0x02;
	dart_r[3] = r;
	dart_r[5] = dtr | 0x0A | ((t->c_cflag & CSIZE) << 1);
}

void tty_setup(uint_fast8_t minor, uint_fast8_t flags)
{
	dart_setup(minor, flags);
	dart_otir(DART0_BASE + 4 + 8 * (minor - 1));
}

int tty_carrier(uint_fast8_t minor)
{
        uint8_t c;
        uint8_t port;

        if (minor == 1) {
        	DARTA_C = 0;
        	c = DARTA_C;
	} else {
		DARTB_C = 0;
		c = DARTB_C;
	}
	c = in(port);
	if (c & 0x08)
		return 1;
	return 0;
}

void tty_pollirq(void)
{
	static uint8_t old_ca[2];

	while(sio_rxl[0])
		tty_inproc(1, sioa_rx_get());
	if (sio_dropdcd[0]) {
		sio_dropdcd[0] = 0;
		tty_carrier_drop(1);
	}
	if (((old_ca[0] ^ sio_state[0]) & sio_state[0]) & 8)
		tty_carrier_raise(1);
	old_ca[0] = sio_state[0];
	if (sio_txl[0] < 64 && (sleeping & (1 << 1))) {
		sleeping &= ~(1 << 1);
		tty_outproc(1);
	}

	while(sio_rxl[1])
		tty_inproc(2, siob_rx_get());
	if (sio_dropdcd[1]) {
		sio_dropdcd[1] = 0;
		tty_carrier_drop(2);
	}
	if (((old_ca[1] ^ sio_state[1]) & sio_state[1]) & 8)
		tty_carrier_raise(2);
	old_ca[1] = sio_state[1];
	if (sio_txl[1] < 64 && (sleeping & (1 << 2))) {
		sleeping &= ~(1 << 2);
		tty_outproc(2);
	}
}

void tty_putc(uint_fast8_t minor, uint_fast8_t c)
{
	if (minor == 1)
		sioa_txqueue(c);
	else
		siob_txqueue(c);
}

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
	if (sio_txl[minor] < 128)
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
	/* Can't use the normal paths as we must survive interrupts off */
	irqflags_t irq = di();

	while(!(DARTA_C & 0x04));
	DARTA_D = c;

	if (c == '\n')
		kputchar('\r');

	irqrestore(irq);
}
