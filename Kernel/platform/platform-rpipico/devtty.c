#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <vt.h>
#include <tty.h>
#include "rawuart.h"
#include "picosdk.h"
#include <pico/multicore.h>
#include "core1.h"

static uint8_t ttybuf[TTYSIZ*NUM_DEV_TTY];

struct s_queue ttyinq[NUM_DEV_TTY+1] = { /* ttyinq[0] is never used */
	{ 0,         0,         0,         0,      0, 0        },
	{ &ttybuf[0],    &ttybuf[0],    &ttybuf[0],    TTYSIZ, 0, TTYSIZ/2 },
#if NUM_DEV_TTY_USB >= 1
	{ &ttybuf[TTYSIZ*1],    &ttybuf[TTYSIZ*1],    &ttybuf[TTYSIZ*1],    TTYSIZ, 0, TTYSIZ/2 },
#endif
#if NUM_DEV_TTY_USB >= 2
	{ &ttybuf[TTYSIZ*2],    &ttybuf[TTYSIZ*2],    &ttybuf[TTYSIZ*2],    TTYSIZ, 0, TTYSIZ/2 },
#endif
#if NUM_DEV_TTY_USB >= 3
	{ &ttybuf[TTYSIZ*3],    &ttybuf[TTYSIZ*3],    &ttybuf[TTYSIZ*3],    TTYSIZ, 0, TTYSIZ/2 },
#endif
#if NUM_DEV_TTY_USB >= 4
	{ &ttybuf[TTYSIZ*4],    &ttybuf[TTYSIZ*4],    &ttybuf[TTYSIZ*4],    TTYSIZ, 0, TTYSIZ/2 },
#endif
};

tcflag_t termios_mask[NUM_DEV_TTY+1] = {
    0,
    _CSYS,
#if NUM_DEV_TTY_USB >= 1
	_CSYS,
#endif
#if NUM_DEV_TTY_USB >= 2
	_CSYS,
#endif
#if NUM_DEV_TTY_USB >= 3
	_CSYS,
#endif
#if NUM_DEV_TTY_USB >= 4
	_CSYS,
#endif
};

/* Output for the system console (kprintf etc) */
void kputchar(uint_fast8_t c)
{
    if (c == '\n')
        uart1_putc('\r');
    //usbconsole_putc_blocking(minor(TTYDEV), c);
    //usbconsole_putc_debug(c);
    uart1_putc(c);
}

void tty_putc(uint_fast8_t minor, uint_fast8_t c)
{

    if (minor == 1)
    {
        uart1_putc(c);
    }
    else
    {
        usbconsole_putc(minor, c);
    }
}

ttyready_t tty_writeready(uint_fast8_t minor)
{
    if (minor == 1)
    {
        return uart1_ready() ? TTY_READY_NOW : TTY_READY_SOON;
    }
    else
    {
        if (usbconsole_is_writable(minor))
        {
            return TTY_READY_NOW;
        }
        return TTY_READY_SOON;
    }
}

/* For the moment */
int tty_carrier(uint_fast8_t minor)
{
    return 1;
}

void tty_sleeping(uint_fast8_t minor)
{
    if(minor != 1)
    {
        usbconsole_setsleep(minor, true);
    }
}
void tty_data_consumed(uint_fast8_t minor) {}
void tty_setup(uint_fast8_t minor, uint_fast8_t flags) {}

void tty_interrupt(void)
{
    int c;
    while ((c = uart1_getc()) >= 0)
    {
        tty_inproc(1, (char)c);
    }
#if NUM_DEV_TTY_USB > 0
    uint8_t cbuf[8];
    int w = usbconsole_read(cbuf, sizeof(cbuf));
    for (int i = 0; i < w; i += 2)
    {
        tty_inproc(cbuf[i], cbuf[i + 1]);
    }
#endif
}
/* vim: sw=4 ts=4 et: */

