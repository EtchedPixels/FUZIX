#include <kernel.h>
#include <kdata.h>
#include <stdbool.h>
#include <tinysd.h>
#include <z80retro.h>

void sd_spi_fast(void)
{
}

/* We only do slow.. */
void sd_spi_slow(void)
{
}

static const uint8_t spi_mask[] = {
    0x01, 0x05, 0x09, 0x11, 0x21, 0x41
};

__sfr __at 0x64	spi_cs;

void spi_select_port(uint8_t port)
{
    spi_cs = spi_mask[port];
}

void sd_spi_raise_cs(void)
{
    spi_cs = 0x01;
}

/* SD is on SPI 1 */
void sd_spi_lower_cs(void)
{
    spi_select_port(1);
}

