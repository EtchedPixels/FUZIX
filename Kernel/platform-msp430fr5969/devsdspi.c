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

void sd_spi_clock(bool go_fast)
{
	UCB0CTLW0 |= UCSWRST;
	UCB0BRW = go_fast ? 1 : 20;
	UCB0CTLW0 &= ~UCSWRST;
}

void sd_spi_raise_cs(void)
{
	P3OUT |= BIT4;
}

void sd_spi_lower_cs(void)
{
	P3OUT &= ~BIT4;
}

void sd_spi_transmit_byte(uint8_t b)
{
	UCB0TXBUF = b;
	while (!(UCB0IFG & UCTXIFG))
		;
}

uint8_t sd_spi_receive_byte(void)
{
	UCB0TXBUF = 0xff; // force MOSI high while waiting

	while (!(UCB0IFG & UCTXIFG))
		;
	while (!(UCB0IFG & UCRXIFG))
		;

	return UCB0RXBUF;
}


bool sd_spi_receive_sector(void)
{
	uint8_t* addr = blk_op.addr;
	int i;
	for (i=0; i<512; i++)
		addr[i] = sd_spi_receive_byte();
	return 0;
}

bool sd_spi_transmit_sector(void)
{
	uint8_t* addr = blk_op.addr;
	int i;
	for (i=0; i<512; i++)
		sd_spi_transmit_byte(addr[i]);
	return 0;
}
