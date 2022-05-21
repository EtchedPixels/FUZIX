/*
 *	This is fairly simple: tty1 is the console. We can't use aux as in
 *	CP/M 2.2 there is no status info for it as we need.
 */
#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <tty.h>
#include <devtty.h>
#include <cpm.h>
#include <sysmod.h>

static char tbuf1[TTYSIZ];
static char tbuf2[TTYSIZ];

struct s_queue ttyinq[NUM_DEV_TTY + 1] = {	/* ttyinq[0] is never used */
	{NULL, NULL, NULL, 0, 0, 0},
	{tbuf1, tbuf1, tbuf1, TTYSIZ, 0, TTYSIZ / 2},
	{tbuf2, tbuf2, tbuf2, TTYSIZ, 0, TTYSIZ / 2},
};

/* Until we wire up the CP/M hooks */
tcflag_t termios_mask[NUM_DEV_TTY + 1] = {
	0,
	_CSYS,
	_CSYS
};

/* Write to system console */
void kputchar(char c)
{
	/* handle CRLF */
	if (c == '\n')
		tty_putc(1, '\r');
	tty_putc(1, c);
}

/*
 *	CP/M check if the tty is ready. Not something we can do properly
 *	in CP/M 2.2.
 */
char tty_writeready(uint8_t minor)
{
	if (cpm_busy)
		return TTY_READY_SOON;
	if (minor == 1)
		return sysmod_conost() ? TTY_READY_NOW : TTY_READY_SOON;
	else
		return sysmod_auxost() ? TTY_READY_NOW : TTY_READY_SOON;
}

void tty_putc(uint8_t minor, unsigned char c)
{
	cpm_busy++;
	if (minor == 1) {
		if (info->features & FEATURE_CONDI) {
			irqflags_t irq = di();
			cpm_conout(c);
			irqrestore(irq);
		} else
			cpm_conout(c);
	} else
		cpm_punch(c);
	cpm_busy--;
}

void tty_sleeping(uint8_t minor)
{
	used(minor);
}

/* Called every timer tick */
void tty_pollirq(void)
{
	uint8_t c;
	/* CP/M is not re-entrant by guarantee */
	if (cpm_busy)
		return;

	while (cpm_const() && !fullq(&ttyinq[1])) {
		c = cpm_conin();
		tty_inproc(1, c);
	}
	if (info->features & FEATURE_AUX) {
		while (sysmod_auxist() && !fullq(&ttyinq[2])) {
			c = cpm_reader();
			tty_inproc(2, c);
		}
	}
}

void tty_setup(uint8_t minor, uint8_t flags)
{
	if (minor == 1)
		ttydata[1].termios.c_cflag =
				sysmod_conconf(ttydata[1].termios.c_cflag);
	else
		ttydata[2].termios.c_cflag =
				sysmod_auxconf(ttydata[2].termios.c_cflag);
}

int tty_carrier(uint8_t minor)
{
	used(minor);
	return 1;
}

void tty_data_consumed(uint8_t minor)
{
}

int my_tty_open(uint8_t minor, uint16_t flag)
{
	if (minor == 2 && !(info->features & FEATURE_AUX)) {
		udata.u_error = ENXIO;
		return -1;
	}
	return tty_open(minor, flag);
}

static uint8_t tcount;

void plt_interrupt(void)
{
	tty_pollirq();
	tcount++;
	if (tcount == info->tickdivider) {
		tcount = 0;
		timer_interrupt();
		poll_input();
	}
}
