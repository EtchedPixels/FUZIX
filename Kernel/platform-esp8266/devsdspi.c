#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <timer.h>
#include <stdbool.h>
#include <blkdev.h>
#include <eagle_soc.h>
#include "esp8266_peri.h"
#include "config.h"

void sd_rawinit(void)
{
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDO_U, 2);
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTCK_U, 2);
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U, 2);
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTMS_U, 2);

    SPI1C = 0;
    SPI1U = SPIUMOSI | SPIUMISO | SPIUDUPLEX;
    SPI1U1 = (7 << SPILMOSI) | (7 << SPILMISO);
    SPI1C1 = 0;
	SPI1S = 0;
}

void sd_spi_clock(bool go_fast)
{
	SPI1CLK = 0x7FFFF020;
}

void sd_spi_raise_cs(void)
{
	//GPIO_REG_WRITE(GPIO_ENABLE_W1TS_ADDRESS, 1<<14);
	SPI1U &= ~(SPIUCSSETUP | SPIUCSHOLD);
}

void sd_spi_lower_cs(void)
{
	//GPIO_REG_WRITE(GPIO_ENABLE_W1TC_ADDRESS, 1<<15);
	SPI1U |= (SPIUCSSETUP | SPIUCSHOLD);
}

static uint8_t xmit_recv(uint8_t b)
{
	kprintf("send %x ", b);
	while (SPI1CMD & SPIBUSY)
		;

	kprintf(".");
	//const uint32_t mask = ~((SPIMMOSI << SPILMOSI) | (SPIMMISO << SPILMISO));
    //SPI1U1 = ((SPI1U1 & mask) | ((7 << SPILMOSI) | (7 << SPILMISO)));
	
	SPI1W0 = b;
	SPI1CMD |= SPIBUSY;

	while (SPI1CMD & SPIBUSY)
		;

	b = SPI1W0 & 0xff;
	kprintf("recv %x\n", b);
	return b;
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

/* vim: sw=4 ts=4 et: */

