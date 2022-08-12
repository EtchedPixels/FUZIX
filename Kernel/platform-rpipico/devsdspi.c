#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <timer.h>
#include <stdbool.h>
#include <stdlib.h>
#include <blkdev.h>
#include "dev/devsd.h"
#include "picosdk.h"
#include "globals.h"
#include "config.h"
#include <hardware/spi.h>

void sd_rawinit(void)
{

#ifndef CONFIG_SD_ALT

// This is the standard pinning as per David Given's prototype.

    gpio_init_mask(0xf << 12);
    gpio_set_function(12, GPIO_FUNC_SPI); // MISO
    gpio_set_function(13, GPIO_FUNC_SIO); // CS
    gpio_set_function(14, GPIO_FUNC_SPI); // SCK
    gpio_set_function(15, GPIO_FUNC_SPI); // MOSI
  #define SDCARD_CS_GP 13
    gpio_set_dir(SDCARD_CS_GP, true);

#else

// Alternative pinning as per the Cytron Maker Pi Pico schematic from e.g.
// https://www.electrokit.com/uploads/productfile/41018/MAKER-PI-PICO%20v1.2.0%20Schematic.pdf

    gpio_init_mask(0x27 << 10);
    gpio_set_function(10, GPIO_FUNC_SPI); // SCK
    gpio_set_function(11, GPIO_FUNC_SPI); // MOSI
    gpio_set_function(12, GPIO_FUNC_SPI); // MISO
    gpio_set_function(15, GPIO_FUNC_SIO); // CS
  #define SDCARD_CS_GP 15
    gpio_set_dir(SDCARD_CS_GP, true);
#endif

    spi_init(spi1, 250000);
    spi_set_format(spi1, 8, 0, 0, SPI_MSB_FIRST);
}

void sd_spi_clock(bool go_fast)
{
    spi_set_baudrate(spi1, go_fast ? 4000000 : 250000);
}

void sd_spi_raise_cs(void)
{
    gpio_put(1<<SDCARD_CS_GP, true);
}

void sd_spi_lower_cs(void)
{
    gpio_put(1<<SDCARD_CS_GP, false);
}

void sd_spi_transmit_byte(uint_fast8_t b)
{
    spi_write_blocking(spi1, (uint8_t*) &b, 1);
}

uint_fast8_t sd_spi_receive_byte(void)
{
    uint8_t b;
    spi_read_blocking(spi1, 0xff, (uint8_t*) &b, 1);
    return b;
}

bool sd_spi_receive_sector(void)
{
    spi_read_blocking(spi1, 0xff, (uint8_t*) blk_op.addr, 512);
	return 0;
}

bool sd_spi_transmit_sector(void)
{
    spi_write_blocking(spi1,  (uint8_t*) blk_op.addr, 512);
	return 0;
}

/* vim: sw=4 ts=4 et: */

