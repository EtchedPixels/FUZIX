#include <stdint.h>
#include <stdbool.h>

#include <hardware/tiva_pinmap.h>
#include <tiva_gpio.h>
#include <tm4c_gpio.h>

#include <kernel.h>
#include <kdata.h>
#include <devsd.h>
#include <blkdev.h>

#include "cpu.h"
#include "types.h"

#include "tm4c129x.h"

#include "ssi.h"

void sd_spi_clock(bool go_fast)
{
}

void sd_spi_raise_cs(void)
{
  irqflags_t fl = __hard_di();

  tiva_gpiowrite(GPIO_PORTQ | GPIO_PIN_1, true);
  tiva_gpiowrite(GPIO_PORTH | GPIO_PIN_4, true);
  __hard_irqrestore(fl);
}

void sd_spi_lower_cs(void)
{
  irqflags_t fl = __hard_di();

  tiva_gpiowrite(GPIO_PORTQ | GPIO_PIN_1, true);
  tiva_gpiowrite(GPIO_PORTH | GPIO_PIN_4, false);
  __hard_irqrestore(fl);
}

void sd_spi_transmit_byte(uint_fast8_t byte)
{
  irqflags_t fl = __hard_di();

  ssi_transfer(SDCARD_SSI_PORT, &byte, NULL, 1U);
  __hard_irqrestore(fl);
}

uint_fast8_t sd_spi_receive_byte(void)
{
  uint8_t byte = 0U;
  irqflags_t fl = __hard_di();

  ssi_transfer(SDCARD_SSI_PORT, NULL, &byte, 1U);
  __hard_irqrestore(fl);
  return byte;
}

bool sd_spi_receive_sector(void)
{
  irqflags_t fl = __hard_di();

  ssi_transfer(SDCARD_SSI_PORT, NULL, blk_op.addr, 512U);
  __hard_irqrestore(fl);
  return true;
}

bool sd_spi_transmit_sector(void)
{
  irqflags_t fl = __hard_di();

  ssi_transfer(SDCARD_SSI_PORT, blk_op.addr, NULL, 512U);
  __hard_irqrestore(fl);
  return true;
}
