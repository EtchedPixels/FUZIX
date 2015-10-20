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

static uint8_t xmit_recv(uint8_t b)
{
	UCB0TXBUF = b;
	while (!(UCB0IFG & (UCTXIFG|UCRXIFG)))
		;
	return UCB0RXBUF;
}

void sd_spi_transmit_byte(uint8_t b)
{
	xmit_recv(b);
}

uint8_t sd_spi_receive_byte(void)
{
	return xmit_recv(0xff);
}


bool sd_spi_receive_sector(void)
{
	uint8_t* addr = blk_op.addr;
	uint8_t* endaddr = addr + 512;

	while (addr != endaddr)
		*addr++ = xmit_recv(0xff);
	return 0;
}

bool sd_spi_transmit_sector(void)
{
	uint8_t* addr = blk_op.addr;
	uint8_t* endaddr = addr + 512;

	while (addr != endaddr)
		xmit_recv(*addr++);
	return 0;
}
