#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <vt.h>
#include <tty.h>
#include "picosdk.h"
#include <pico/multicore.h>
#include "core1.h"

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
        usbconsole_putc_blocking('\r');
    usbconsole_putc_blocking(c);
}

void tty_putc(uint_fast8_t minor, uint_fast8_t c)
{
	kputchar(c);
}

ttyready_t tty_writeready(uint_fast8_t minor)
{
    return usbconsole_is_writable() ? TTY_READY_NOW : TTY_READY_SOON;
}

/* For the moment */
int tty_carrier(uint_fast8_t minor)
{
    return 1;
}

void tty_sleeping(uint_fast8_t minor) {}
void tty_data_consumed(uint_fast8_t minor) {}
void tty_setup(uint_fast8_t minor, uint_fast8_t flags) {}

/* vim: sw=4 ts=4 et: */

