/*
 *	SPI glue for the SocZ80. Based on Will Sowerbutts's UZI180 for SocZ80
 *	and his implementation for the N8VEM Mark 4
 */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <timer.h>
#include <stdbool.h>
#include <blkdev.h>
#include "config.h"
#include "externs.h"
#include "msp430fr5969.h"

#define SPI_MODE_0 (UCCKPH)
#define SPI_MODE_1 (0)
#define SPI_MODE_2 (UCCKPH|UCCKPL)
#define SPI_MODE_3 (UCCKPL)

void sd_rawinit(void)
{
	/* The SD card is an SPI device with CS on P3.4. All our SPI devices
	 * are going to be on UCB0. */

	P3SEL1 &= ~BIT4;
	P3SEL0 &= ~BIT4;
	P3DIR |= BIT4;         // set pin as output
	P3OUT &= ~BIT4;        // lower CS

	/* Disable UCB0 while we set it up. */

	UCB0CTLW0 = UCSWRST;

	/* Connect up SIMO, MISO. */

	P1SEL1 |= BIT6|BIT7;
	P1SEL0 &= ~(BIT6|BIT7);

	/* Connect up SCLK. */

	P2SEL1 |= BIT2;
	P2SEL0 &= ~BIT2;

	/* Set 400kHz output clock (SMCLK is 8MHz, divided by 20). */

	UCB0BRW = 20;

	/* Interrupts off. */

	UCB0IE &= ~(UCTXIE | UCRXIE);

	/* Initialise UCB0. */

	UCB0CTLW0 = UCSWRST
		| UCSSEL__SMCLK    // use SMCLK
		| UCMODE_0         // 3-wire SPI mode
		| UCMST            // master
		| UCMSB            // send MSB first
		| UCSYNC           // synchronous (SPI rather than I2C)
		| SPI_MODE_0
		;

	/* Enable. */

	UCB0CTLW0 &= ~UCSWRST;
}
