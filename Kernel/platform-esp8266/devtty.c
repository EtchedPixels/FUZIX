#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <devtty.h>
#include <vt.h>
#include <tty.h>

extern void ets_putc(char c);

static uint8_t ttybuf[TTYSIZ];

struct s_queue ttyinq[NUM_DEV_TTY+1] = { /* ttyinq[0] is never used */
	{ 0,         0,         0,         0,      0, 0        },
	{ ttybuf,    ttybuf,    ttybuf,    TTYSIZ, 0, TTYSIZ/2 },
};

tcflag_t termios_mask[NUM_DEV_TTY+1] = { 0, _CSYS };

/* Output for the system console (kprintf etc) */
void kputchar(uint_fast8_t c)
{
	if (c == '\n')
		ets_putc('\r');
	ets_putc(c);
}

void tty_putc(uint_fast8_t minor, uint_fast8_t c)
{
	kputchar(c);
}

void tty_sleeping(uint_fast8_t minor)
{
}

ttyready_t tty_writeready(uint_fast8_t minor)
{
	return TTY_READY_NOW;
}

void tty_setup(uint_fast8_t minor, uint_fast8_t flags)
{
	/* Already done on system boot (because we only support one TTY so far). */
}

/* For the moment */
int tty_carrier(uint_fast8_t minor)
{
    return 1;
}

void tty_data_consumed(uint_fast8_t minor)
{
}

void tty_interrupt(void)
{
	//tty_inproc(minor(BOOT_TTY), UCA0RXBUF);
}

