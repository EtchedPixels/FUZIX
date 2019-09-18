#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <tty.h>
#include <devtty.h>

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
	CSIZE|CSTOPB|PARENB|PARODD|_CSYS,
	CSIZE|CSTOPB|PARENB|PARODD|_CSYS,
};

uint8_t sio_r[] = {
	0x03, 0xC1,
	0x04, 0xC4,
	0x05, 0xEA
};

static void sio2_setup(uint_fast8_t minor, uint_fast8_t flags)
{
	struct termios *t = &ttydata[minor].termios;
	uint8_t r;

	used(flags);

	/* Set bits per character */
	sio_r[1] = 0x01 | ((t->c_cflag & CSIZE) << 2);

	r = 0xC4;

	t->c_cflag &= ~CBAUD;
	t->c_cflag |= B115200;

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
	sio2_setup(minor, flags);
	/* Actually its + (minor - 1) + 2: the control port */
	sio2_otir(SIO0_BASE + minor + 1);
}

int tty_carrier(uint_fast8_t minor)
{
	/* These are held low on the board 8( */
	return 0;
}

static uint8_t timer_state;

static void timer_tick(void)
{
	timer_state++;
	/* We have an 8Hz source so we need to insert two extra ticks
	   per 8 to make it up. We insert them at xxxxx000 and xxxxx100 */
	if (!(timer_state & 3))
		timer_interrupt();
	timer_interrupt();
}

void tty_poll(void)
{
	static uint8_t old_ca, old_cb;
	uint8_t ca, cb;
	uint8_t progress;

	/* Check for an interrupt */
	SIOA_C = 0;
	if (!(SIOA_C & 2))
		return;

	do {
		progress = 0;
		SIOA_C = 0;		// read register 0
		ca = SIOA_C;
		/* Input pending */
		if (ca & 1) {
			progress = 1;
			tty_inproc(1, SIOA_D);
		}
		/* Output pending */
		if ((ca & 4) && (sleeping & 2)) {
			tty_outproc(2);
			sleeping &= ~2;
			SIOA_C = 5 << 3;	// reg 0 CMD 5 - reset transmit interrupt pending
		}
		/* Carrier changed on A: this is a timer interrupt from
		   the external square wave generator */
		if ((ca ^ old_ca) & 8) {
			if (ca & 8)
				timer_tick();
		}
		/* ACK any break or error events */
		SIOA_C = 2 << 3;

		SIOB_C = 0;		// read register 0
		cb = SIOB_C;
		if (cb & 1) {
			tty_inproc(2, SIOB_D);
			progress = 1;
		}
		if ((cb & 4) && (sleeping & 8)) {
			tty_outproc(3);
			sleeping &= ~8;
			SIOB_C = 5 << 3;	// reg 0 CMD 5 - reset transmit interrupt pending
		}

		/* ACK any break or error events */
		SIOB_C = 2 << 3;

	} while(progress);
}

void tty_putc(uint_fast8_t minor, uint_fast8_t c)
{
	uint8_t port = SIO0_BASE + minor - 1;
	out(port, c);
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
	irqflags_t irq;
	uint8_t c;
	uint8_t port;

	irq = di();
	port = SIO0_BASE + minor + 1;
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
	while(tty_writeready(TTYDEV - 512) != TTY_READY_NOW);
	if (c == '\n')
		tty_putc(TTYDEV - 512, '\r');
	while(tty_writeready(TTYDEV - 512) != TTY_READY_NOW);
	tty_putc(TTYDEV - 512, c);
}
