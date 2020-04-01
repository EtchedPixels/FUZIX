#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <tty.h>
#include <devtty.h>

static char tbuf1[TTYSIZ];
static char tbuf2[TTYSIZ];

tcflag_t termios_mask[NUM_DEV_TTY + 1] = {
	0,
	CSIZE|CSTOPB|CBAUD|PARENB|PARODD|CRTSCTS|_CSYS,
	CSIZE|CSTOPB|CBAUD|PARENB|PARODD|CRTSCTS|_CSYS
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

static uint8_t sleeping;

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
	sio_wr5[2 - minor] = sio_r[5];
	sio_flow[2 - minor] = (t->c_cflag & CRTSCTS);
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

void tty_drain_sio(void)
{
	static uint8_t old_ca[2];
	uint8_t c;

	/* Start with port B as it is run the fastest usually */
	while(sio_rxl[1]) {
		c = siob_rx_get();
		tty_inproc(1, c);
	}
	if (sio_dropdcd[1]) {
		sio_dropdcd[1] = 0;
		tty_carrier_drop(1);
	}
	if (((old_ca[1] ^ sio_state[1]) & sio_state[1]) & 8)
		tty_carrier_raise(1);
	old_ca[1] = sio_state[1];
	if (sio_txl[1] < 64 && (sleeping & 2)) {
		sleeping &= ~2;
		tty_outproc(1);
	}

	while(sio_rxl[0]) {
		c = sioa_rx_get();
		tty_inproc(2, c);
	}
	if (sio_dropdcd[0]) {
		sio_dropdcd[0] = 0;
		tty_carrier_drop(2);
	}
	if (((old_ca[0] ^ sio_state[0]) & sio_state[0]) & 8)
		tty_carrier_raise(2);
	old_ca[0] = sio_state[0];
	if (sio_txl[0] < 64 && (sleeping & 4)) {
		sleeping &= ~4;
		tty_outproc(2);
	}

}

void tty_putc(uint8_t minor, unsigned char c)
{
	if (minor == 1)
		siob_txqueue(c);
	else
		sioa_txqueue(c);
}

/* We will need this for SIO once we implement flow control signals */
void tty_sleeping(uint8_t minor)
{
	sleeping |= (1 << minor);
}

/* Be careful here. We need to peek at RR but we must be sure nobody else
   interrupts as we do this. Really we want to switch to irq driven tx ints
   on this platform I think. Need to time it and see

   An asm common level tty driver might be a better idea

   Need to review this we should be ok as the IRQ handler always leaves
   us pointing at RR0 */

ttyready_t tty_writeready(uint8_t minor)
{
	if (minor == 1 && sio_txl[1] >= 127)
		return TTY_READY_SOON;
	if (minor == 2 && sio_txl[0] >= 127)
		return TTY_READY_SOON;
	return TTY_READY_NOW;
}

void tty_data_consumed(uint8_t minor)
{
	used(minor);
}

/* kernel writes to system console -- never sleep! */
void kputchar(char c)
{
	/* Can't use the normal paths as we must survive interrupts off */
	/* FIXME: would be nicer to just disable tx int and re-enable it ? */
	irqflags_t irq = di();
	while(!(SIOB_C & 0x04));
	SIOB_D = c;
	if (c == '\n')
		kputchar('\r');
	irqrestore(irq);
}
