#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <devtty.h>
#include <vt.h>
#include <tty.h>
#include "globals.h"
#include "soc/uart_struct.h"
#include "hal/uart_ll.h"

#define UART UART0
static uint8_t ttybuf[TTYSIZ];

struct s_queue ttyinq[NUM_DEV_TTY + 1] = {	/* ttyinq[0] is never used */
	{ 0, 0, 0, 0, 0, 0 },
	{ ttybuf, ttybuf, ttybuf, TTYSIZ, 0, TTYSIZ / 2 },
};

tcflag_t termios_mask[NUM_DEV_TTY + 1] = {
	0,
	_CSYS | CBAUD | CSTOPB | PARENB | PARODD | CSIZE
};

static unsigned int tty_stalled;

void tty_putc(uint_fast8_t minor, uint_fast8_t c)
{
	kputchar(c);
}

void tty_sleeping(uint_fast8_t minor)
{
}

ttyready_t tty_writeready(uint_fast8_t minor)
{
    return TTY_READY_SOON;
}

/* For the moment */
int tty_carrier(uint_fast8_t minor)
{
	return 1;
}

/* Called when the tty layer has eaten input - try and empty more FIFO */
void tty_data_consumed(uint_fast8_t minor)
{
}

/* A tty receive interrupt (we don't enable other types at the moment). Empty the FIFO if we can */
void tty_interrupt(void)
{
}

void tty_setup(uint_fast8_t minor, uint_fast8_t flags)
{
}

/* vim: sw=4 ts=4 et: */
