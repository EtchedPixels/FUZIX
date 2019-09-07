/*
 *	The SPX302 has multiple serial ports but the 68302 built in ones are
 *	wired for RS485 so for now ignore them. Some day they'll be handy for
 *	TCP/IP ;-)
 *
 *	This wants rewriting once it is all stable so that the serial interrupt
 *	isn't masked by the kernel and drives queues.
 */
#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <devtty.h>
#include <tty.h>

static unsigned char tbuf1[TTYSIZ];
static unsigned char tbuf2[TTYSIZ];

#define UART_MRA	0x00
#define UART_SRA	0x02
#define UART_CSRA	0x02
#define UART_CRA	0x04
#define UART_RHRA	0x06
#define UART_THRA	0x06
#define UART_IPCR	0x08
#define UART_ACR	0x08
#define UART_ISR	0x0A
#define UART_IMR	0x0A
#define UART_CTU	0x0C
#define UART_CTUR	0x0C
#define UART_CTL	0x0E
#define UART_CTLR	0x0E
#define UART_MRB	0x10
#define UART_SRB	0x12
#define UART_CSRB	0x12
#define UART_CRB	0x14
#define UART_RHRB	0x16
#define UART_THRB	0x16
#define UART_IVR	0x18
#define UART_OPCR	0x1A
#define UART_STARTCTR	0x1C
#define UART_SETOPR	0x1C
#define UART_STOPCTR	0x1E
#define UART_CLROPR	0x1E

struct s_queue ttyinq[NUM_DEV_TTY + 1] = {	/* ttyinq[0] is never used */
	{NULL, NULL, NULL, 0, 0, 0},
	{tbuf1, tbuf1, tbuf1, TTYSIZ, 0, TTYSIZ / 2},
	{tbuf2, tbuf2, tbuf2, TTYSIZ, 0, TTYSIZ / 2},
};

static uint8_t sleeping;

/* For now */
tcflag_t termios_mask[NUM_DEV_TTY + 1] = {
	0,
	_CSYS,
	_CSYS
};

static volatile uint8_t *uart_base = (volatile uint8_t *)0x300020;

#define GETB(x)		(uart_base[(x)])
#define PUTB(x,y)	uart_base[(x)] = (y)

/* Output for the system console (kprintf etc). Polled. */
void kputchar(uint8_t c)
{
	if (c == '\n') {
		while(!(GETB(UART_SRA) &  4));
		PUTB(UART_THRA, '\r');
	}
	while(!(GETB(UART_SRA) &  4));
	PUTB(UART_THRA, c);
}

ttyready_t tty_writeready(uint8_t minor)
{
	uint8_t c = GETB(0x10 * (minor - 1) + UART_SRA);
	return (c & 4) ? TTY_READY_NOW : TTY_READY_SOON; /* TX DATA empty */
}

void tty_putc(uint8_t minor, unsigned char c)
{
	PUTB(0x10 * (minor - 1) + UART_THRA, c);
}

void tty_setup(uint8_t minor, uint8_t flags)
{
	/* TODO */
}

int tty_carrier(uint8_t minor)
{
	return 1;
}

void tty_sleeping(uint8_t minor)
{
	sleeping |= 1 << minor;
}

void tty_data_consumed(uint8_t minor)
{
}

static void tty_interrupt(uint8_t r)
{
	if (r & 0x02) {
		r = GETB(UART_RHRA);
		tty_inproc(1,r);
	}	
	if (r & 0x01) {
		if (sleeping & 2)
			tty_outproc(1);
	}
	if (r & 0x04) {
		/* How to clear break int ? */
	}
	if (r & 0x20) {
		r = GETB(UART_RHRB);
		tty_inproc(2,r);
	}	
	if (r & 0x10) {
		if (sleeping & 4)
			tty_outproc(2);
	}
	if (r & 0x40) {
		/* How to clear break int ? */
	}
}

void duart_interrupt(void)
{
	uint8_t r = GETB(UART_ISR);
	tty_interrupt(r);
}

void platform_interrupt(void)
{
	timer_interrupt();
}
