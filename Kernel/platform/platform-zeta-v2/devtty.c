#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <tty.h>
#include <devtty.h>
#include <zeta-v2.h>

static char tbuf1[TTYSIZ];

unsigned char uart0_type;

struct  s_queue  ttyinq[NUM_DEV_TTY+1] = {       /* ttyinq[0] is never used */
	{NULL,	NULL,	NULL,	0,	0,	0},
	{tbuf1,	tbuf1,	tbuf1,	TTYSIZ,	0,	TTYSIZ/2}
};

tcflag_t termios_mask[NUM_DEV_TTY + 1] = {
	0,
	CSIZE|CBAUD|CSTOPB|PARENB|PARODD|_CSYS,
};


uint16_t divisor_table[16] = {
	0, UART_CLOCK / 16 / 50, UART_CLOCK / 16 / 75, UART_CLOCK / 16 / 110,
	UART_CLOCK / 16 / 134, UART_CLOCK / 16 / 150, UART_CLOCK / 16 / 300,
	UART_CLOCK / 16 / 600, UART_CLOCK / 16 / 1200, UART_CLOCK / 16 / 2400,
	UART_CLOCK / 16 / 4800, UART_CLOCK / 16 / 9600,
	UART_CLOCK / 16 / 19200, UART_CLOCK / 16 / 38400,
	UART_CLOCK / 16 / 57600, UART_CLOCK / 16 / 115200
};

void tty_setup(uint8_t minor, uint8_t flags)
{
	uint16_t b;
	uint8_t lcr = 0;
	if (minor == 1) {
		b = ttydata[minor].termios.c_cflag & CBAUD;
		if (boot_from_rom && b > 0 && b < 16) {
			UART0_LCR = 0x80;	/* LCR = DLAB ON */
			UART0_DLL = divisor_table[b] & 0xFF;
			UART0_DLH = divisor_table[b] >> 8;
		}
		/* word length 5(00), 6(01), 7(10), or 8(11) */
		lcr = (ttydata[minor].termios.c_cflag & CSIZE) >> 4;
		/* stop bits 1(0), or 2(1) */
		lcr |= (ttydata[minor].termios.c_cflag & CSTOPB) >> 4;
		/* parity disable(0), or enable(1) */
		lcr |= (ttydata[minor].termios.c_cflag & PARENB) >> 5;
		/* parity odd(0), or even(1) */
		lcr |= ((ttydata[minor].termios.c_cflag & PARODD) ^ PARODD) >> 5;
		UART0_LCR = lcr;
		UART0_MCR = 0x03; /* DTR = ON, RTS = ON */
	}
}

int tty_carrier(uint8_t minor)
{
	uint8_t c;
	if (minor == 1) {
		c = UART0_MSR;
		return (c & 0x80) ? 1 : 0; /* test DCD */
	}
	return 1;
}

void tty_pollirq_uart0(void)
{
	uint8_t iir, msr, lsr;
	while (true) {
		iir = UART0_IIR;
		lsr = UART0_LSR;
		/* IRR bits
		 * 3 2 1 0
		 * -------
		 * x x x 1     no interrupt pending
		 * 0 1 1 0  6  LSR changed -- read the LSR
		 * 0 1 0 0  4  receive FIFO >= threshold
		 * 1 1 0 0  C  received data sat in FIFO for a while
		 * 0 0 1 0  2  transmit holding register empty
		 * 0 0 0 0  0  MSR changed -- read the MSR
		 */
		switch (iir & 0x0F) {
		case 0x0: /* MSR changed */
		case 0x2: /* transmit register empty */
			msr = UART0_MSR;
			if ((msr & 0x10) && (lsr & 0x20)){
				/* CTS high, transmit reg empty */
				tty_outproc(1);
			}
			/* fall through */
		case 0x6: /* LSR changed */
			/* we already read the LSR register so int has cleared */
			UART0_IER = 0x01; /* enable only receive interrupts */
			break;
		case 0x4: /* receive (FIFO >= threshold) */
		case 0xC: /* receive (timeout waiting for FIFO to fill) */
			while (lsr & 0x01) { /* Data ready */
				tty_inproc(1, UART0_RBR);
				lsr = UART0_LSR;
			}
			break;
		default:
			return;
		}
	}
}

void tty_putc(uint8_t minor, unsigned char c)
{
	if (minor == 1) {
		while(!(UART0_LSR & 0x20));	/* FIXME */
		UART0_THR = c;
	}
}

void tty_sleeping(uint8_t minor)
{
	if (minor == 1) {
		UART0_IER = 0x0B; /* enable all but LSR interrupt */
	}
}

void tty_data_consumed(uint8_t minor)
{
}

ttyready_t tty_writeready(uint8_t minor)
{
	uint8_t c;
	if (minor == 1) {
		c = UART0_MSR;
		if ((ttydata[1].termios.c_cflag & CRTSCTS) && (c & 0x10) == 0) /* CTS not asserted? */
			return TTY_READY_LATER;
		c = UART0_LSR;
		if (c & 0x20) /* THRE? */
			return TTY_READY_NOW;
		return TTY_READY_SOON;
	}
	return TTY_READY_NOW;
}

/* kernel writes to system console -- never sleep! */
void kputchar(char c)
{
    tty_putc(TTYDEV - 512, c);
    if(c == '\n')
        tty_putc(TTYDEV - 512, '\r');
}
