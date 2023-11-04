#include <kernel.h>
#include <sdcard.h>
#include <tinysd.h>

#define	spi_cs	0xC4
#define spi_clk	0xC0

void sd_spi_raise_cs(void)
{
    out(spi_cs, 1);
    out(spi_clk, 1);
}

void sd_spi_lower_cs(void)
{
    out(spi_cs, 0);
    out(spi_clk, 1);
}

void sd_spi_fast(void)
{
}

void sd_spi_slow(void)
{
}

