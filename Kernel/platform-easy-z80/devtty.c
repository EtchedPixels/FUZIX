#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <tty.h>
#include <devtty.h>
#include <easy-z80.h>

static char tbuf1[TTYSIZ];
static char tbuf2[TTYSIZ];

static uint8_t sleeping;

struct s_queue ttyinq[NUM_DEV_TTY + 1] = {	/* ttyinq[0] is never used */
	{NULL, NULL, NULL, 0, 0, 0},
	{tbuf1, tbuf1, tbuf1, TTYSIZ, 0, TTYSIZ / 2},
	{tbuf2, tbuf2, tbuf2, TTYSIZ, 0, TTYSIZ / 2},
};

tcflag_t termios_mask[NUM_DEV_TTY + 1] = {
	0,
	/* FIXME CTS/RTS */
	CSIZE|CBAUD|CSTOPB|PARENB|PARODD|_CSYS,
	/* FIXME CTS/RTS */
	CSIZE|CBAUD|CSTOPB|PARENB|PARODD|_CSYS
};

uint8_t sio_r[] = {
	0x03, 0xC1,
	0x04, 0x44,
	0x05, 0xEA
};

/* We are clocking at 1.8432MHz and x16 or x64 for the low speeds */
static uint16_t siobaud[] = {
	0xC0,	/* 0 */
	0,	/* 50 */
	0,	/* 75 */
	0,	/* 110 */
	0xD6,	/* 134 */
	0xC0,	/* 150 */
	0x60,	/* 300 */
	0xC0,	/* 600 */
	0x60,	/* 1200 */
	0x30,	/* 2400 */
	0x18,	/* 4800 */
	0x0C,	/* 9600 */
	0x06,	/* 19200 */
	0x03,	/* 38400 */
	0x02,	/* 57600 */
	0x01	/* 115200 */
};

static const uint8_t sio2_cmap[3] = {
	0x00,	/* unused */
	0x81,
	0x83,
};

static const uint8_t sio_dmap[3] = {
	0x00,	/* unused */
	0x80,
	0x82,
};

static void sio2_setup(uint8_t minor, uint8_t flags)
{
	struct termios *t = &ttydata[minor].termios;
	uint8_t r;
	uint8_t baud;

	used(flags);

	baud = t->c_cflag & CBAUD;
	if (baud < B134)
		baud = B134;

	/* Set bits per character */
	sio_r[1] = 0x01 | ((t->c_cflag & CSIZE) << 2);

	r = 0xC4;

	if (minor == 1) {
		CTC_CH1 = 0x55;
		CTC_CH1 = siobaud[baud];
	} else {
		CTC_CH2 = 0x55;
		CTC_CH2 = siobaud[baud];
	}
	if (baud >= B600)	/* Use x16 clock and CTC divider */
		r = 0x44;

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

void tty_setup(uint8_t minor, uint8_t flags)
{
	sio2_setup(minor, flags);
	sio2_otir(sio2_cmap[minor]);
}

int tty_carrier(uint8_t minor)
{
        uint8_t c;
        uint8_t port;

	port = sio2_cmap[minor];
	out(port, 0);
	c = in(port);
	if (c & 0x08)
		return 1;
	return 0;
}

void tty_drain_sio(void)
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

void tty_putc(uint8_t minor, unsigned char c)
{
	irqflags_t irqflags = di();

	switch(minor) {
	case 1:
		sioa_txqueue(c);
		break;
	case 2:
		siob_txqueue(c);
		break;
	}
	irqrestore(irqflags);
}

void tty_sleeping(uint8_t minor)
{
	sleeping |= (1 << minor);
}

ttyready_t tty_writeready(uint8_t minor)
{
	if (sio_txl[minor] < 128)
		return TTY_READY_NOW;
	return TTY_READY_SOON;
}

void tty_data_consumed(uint8_t minor)
{
	used(minor);
}

/* kernel writes to system console -- never sleep! */

void kputchar(char c)
{
	/* Can't use the normal paths as we must survive interrupts off */
	irqflags_t irq = di();

	while(!(SIOA_C & 0x04));
	SIOA_D = c;

	if (c == '\n')
		kputchar('\r');

	irqrestore(irq);
}

int rctty_open(uint8_t minor, uint16_t flag)
{
	/* Will need extending if we are support for plug in serial */
	return tty_open(minor, flag);
}
