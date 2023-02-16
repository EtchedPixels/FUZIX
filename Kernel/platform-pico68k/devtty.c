#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <devtty.h>
#include <tty.h>
#include <pico68.h>

#undef  DEBUG			/* Undefine to delete debug code sequences */
static unsigned char tbuf1[TTYSIZ];

struct s_queue ttyinq[NUM_DEV_TTY + 1] = {	/* ttyinq[0] is never used */
	{ NULL, NULL, NULL, 0, 0, 0 },
	{ tbuf1, tbuf1, tbuf1, TTYSIZ, 0, TTYSIZ / 2 },
};

tcflag_t termios_mask[NUM_DEV_TTY + 1] = {
	0,
	CSIZE | CBAUD | CSTOPB | PARENB | PARODD | _CSYS,
};

/* Output for the system console (kprintf etc) */
void kputchar(uint_fast8_t c)
{
	if (c == '\n')
		kputchar('\r');
	while (tty_writeready(1) != TTY_READY_NOW);
	tty_putc(TTYDEV & 0xff, c);
}

ttyready_t tty_writeready(uint_fast8_t minor)
{
	if (acia->ctrl & 2)
		return TTY_READY_NOW;
	return TTY_READY_SOON;
}

void tty_putc(uint_fast8_t minor, uint_fast8_t c)
{
	acia->data = c;		/* Data */
}

void tty_sleeping(uint_fast8_t minor)
{
	used(minor);
}

/*
 *	This function is called whenever the terminal interface is opened
 *	or the settings changed. It is responsible for making the requested
 *	changes to the port if possible. Strictly speaking it should write
 *	back anything that cannot be implemented to the state it selected.
 */
void tty_setup(uint_fast8_t minor, uint_fast8_t flags)
{
	struct termios *t = &ttydata[minor].termios;
	uint8_t r = t->c_cflag & CSIZE;
	if (r <= CS7) {
		t->c_cflag &= ~CSIZE;
		t->c_cflag |= CS7;
	}
	/* 7X1 is allowed and 7X2 but not 8N2 */
	if (r == CS8 && (t->c_cflag & PARENB))
		t->c_cflag &= ~CSTOPB;
	switch (t->c_cflag & (CSIZE | PARENB | PARODD | CSTOPB)) {
	case CS7 | PARENB:
		r = 0x8A;
		break;
	case CS7 | PARENB | PARODD:
		r = 0x8E;
		break;
	case CS7 | PARENB | CSTOPB:
		r = 0x82;
	case CS7 | PARENB | PARODD | CSTOPB:
		r = 0x86;
	case CS8 | CSTOPB:
		r = 0x92;
		break;
	default:
	case CS8:
		r = 0x96;
		break;
	case CS8 | PARENB:
		r = 0x9A;
		break;
	case CS8 | PARENB | PARODD:
		r = 0x9E;
		break;
	}
	acia->ctrl = r;
}

int tty_carrier(uint_fast8_t minor)
{
	return 1;
}

void tty_data_consumed(uint_fast8_t minor)
{
}

void tty_interrupt(void)
{
	if (acia->ctrl & 1)
		tty_inproc(1, acia->data);
}

void plt_interrupt(void)
{
	tty_interrupt();
	if (via->ifr & 0x40) {
		via->t1c_l;	/* Clear interrupt */
		timer_interrupt();
	}
}
