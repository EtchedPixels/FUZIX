#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <vt.h>
#include <tty.h>
#include "picosdk.h"
#include <hardware/uart.h>

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
		uart_putc(uart_default, '\r');
	uart_putc(uart_default, c);
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
//    return (tx_buffer_fill() >= 0x7f) ? TTY_READY_SOON : TTY_READY_NOW;
}

/* For the moment */
int tty_carrier(uint_fast8_t minor)
{
    return 1;
}

void tty_data_consumed(uint_fast8_t minor)
{
}

//static void tty_isr(void* user, struct __exception_frame* ef)
//{
//    while (rx_buffer_fill() != 0)
//    {
//        uint8_t b = U0F;
//        tty_inproc(minor(BOOT_TTY), b);
//    }
//
//    U0IC = 1 << UITO;
//}
//
void tty_setup(uint_fast8_t minor, uint_fast8_t flags)
{
//    irqflags_t irq = di();
//
//    U0C1 = (0x02 << UCTOT)   /* RX timeout threshold */
//        | (1 << UCTOE)       /* RX timeout enable */
//        ;
//
//    U0IC = 0xffff;           /* clear all pending interrupts */
//    U0IE = 1 << UITO;        /* RX timeout enable */
//
//	ets_isr_attach(ETS_UART_INUM, tty_isr, NULL);
//	ets_isr_unmask(1<<ETS_UART_INUM);
//
//    irqrestore(irq);
}

/* vim: sw=4 ts=4 et: */

