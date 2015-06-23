#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <devtty.h>
#include <vt.h>
#include <tty.h>
#include "msp430fr5969.h"

uint8_t tbuf1[TTYSIZ];

struct s_queue ttyinq[NUM_DEV_TTY+1] = { /* ttyinq[0] is never used */
	{ NULL,    NULL,    NULL,    0,        0,       0    },
	{ tbuf1,   tbuf1,   tbuf1,   TTYSIZ,   0,   TTYSIZ/2 },
};

/* Output for the system console (kprintf etc) */
void kputchar(char c)
{
	if (c == '\n')
		tty_putc(0, '\r');
	tty_putc(0, c);
}

void tty_rawinit(void)
{
	/* The tty uses UCA0. */

	/* Connect pins 2.0 and 2.1 to the UART. */
	P2SEL1 |= BIT0 | BIT1;
	P2SEL0 &= ~(BIT0 | BIT1);

	/* Disable high-impedence GPIO mode. */
	PM5CTL0 &= ~LOCKLPM5;

	/* Init UART to 8n1 and CLK=SMCLK (8MHz). */
	UCA0CTLW0 = UCSWRST | UCSSEL__SMCLK;

	/* The values below are simply looked up in table 21-5 on page 562 of
	 * the manual; 9600 baud at 8MHz:
	 * UCOS16=1 UCBRx=52 UCBRFx=1 UCBRSx=0x49
	 */
	UCA0BR0 = 52; /* aka UCBRx lo */
	UCA0BR1 = 0; /* hi */
	UCA0MCTLW = UCOS16 | UCBRF_1; //(1<<4) | (0x49<<8); /* UCOS16, UCBRFx, UCBRSx */

	/* Unreset UART. */
	UCA0CTLW0 &= ~(UCSWRST);
	
	/* Disable RX/TX interrupts. */
	UCA0IE &= ~(UCTXIE | UCRXIE);
}

void tty_putc(uint8_t minor, unsigned char c)
{
	while (!(UCA0IFG & UCTXIFG))
		;

	UCA0TXBUF = c;
}

void tty_sleeping(uint8_t minor)
{
}

ttyready_t tty_writeready(uint8_t minor)
{
	return (UCA0IFG & UCTXIFG) ? TTY_READY_NOW : TTY_READY_LATER;
}

void tty_setup(uint8_t minor)
{
	/* Already done on system boot (because we only support one TTY so far). */
}

/* For the moment */
int tty_carrier(uint8_t minor)
{
    return 1;
}


