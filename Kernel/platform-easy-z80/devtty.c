#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <tty.h>
#include <devtty.h>
#include <easy-z80.h>

static char tbuf1[TTYSIZ];
static char tbuf2[TTYSIZ];
static char tbuf3[TTYSIZ];
static char tbuf4[TTYSIZ];

static uint8_t sleeping;

struct s_queue ttyinq[NUM_DEV_TTY + 1] = {	/* ttyinq[0] is never used */
	{NULL, NULL, NULL, 0, 0, 0},
	{tbuf1, tbuf1, tbuf1, TTYSIZ, 0, TTYSIZ / 2},
	{tbuf2, tbuf2, tbuf2, TTYSIZ, 0, TTYSIZ / 2},
	{tbuf3, tbuf3, tbuf3, TTYSIZ, 0, TTYSIZ / 2},
	{tbuf4, tbuf2, tbuf2, TTYSIZ, 0, TTYSIZ / 2},
};

static tcflag_t uart_mask[4] = {
	_ISYS,
	_OSYS,
	CSIZE|CSTOPB|PARENB|PARODD|_CSYS,
	_LSYS
};

static tcflag_t uartctc_mask[4] = {
	_ISYS,
	/* FIXME: break */
	_OSYS,
	/* FIXME CTS/RTS */
	CSIZE|CBAUD|CSTOPB|PARENB|PARODD|_CSYS,
	_LSYS,
};

tcflag_t *termios_mask[NUM_DEV_TTY + 1] = {
	NULL,
	uartctc_mask,
	uartctc_mask,
	uart_mask,
	uart_mask
};

uint8_t sio_r[] = {
	0x03, 0xC1,
	0x04, 0xC4,
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

static const uint8_t sio2_cmap[5] = {
	0x00,	/* unused */
	0x81,
	0x83,
	0x84,
	0x86
};

static const uint8_t sio_dmap[5] = {
	0x00,	/* unused */
	0x80,
	0x82,
	0x85,
	0x87
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

	t->c_cflag &= CBAUD;
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

typedef uint8_t (*sio_pollfunc_t)(void);

void tty_drain_sio(void)
{
	static uint8_t old_ca[4];
	uint8_t n;
	uint8_t l = 2;

	if (sio1_present)
		l = 4;

	for (n = 0; n < l; n++) {
		while(sio_rxl[n]) {
			switch(n) {
			case 0:
				tty_inproc(1, sioa_rx_get());
				break;
			case 1:
				tty_inproc(2, siob_rx_get());
				break;
			case 2:
				tty_inproc(3, sioc_rx_get());
				break;
			case 3:
				tty_inproc(4, siod_rx_get());
				break;
			}
		}
		if (sio_dropdcd[n]) {
			sio_dropdcd[n] = 0;
			tty_carrier_drop(n + 1);
		}
		if (((old_ca[n] ^ sio_state[n]) & sio_state[n]) & 8)
		tty_carrier_raise(n + 1);
		old_ca[n] = sio_state[n];
		if (sio_txl[n] < 64 && (sleeping & (1 << (n + 1)))) {
			sleeping &= ~(1 << (n + 1));
			tty_outproc(n + 1);
		}
	}
}

void tty_putc(uint8_t minor, unsigned char c)
{
	switch(minor) {
	case 1:
		sioa_txqueue(c);
		break;
	case 2:
		siob_txqueue(c);
		break;
	case 3:
		sioc_txqueue(c);
		break;
	case 4:
		siod_txqueue(c);
		break;
	}
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
	if ((minor == 3 || minor == 4) && !sio1_present) {
		udata.u_error = ENODEV;
		return -1;
	}
	return tty_open(minor, flag);
}
