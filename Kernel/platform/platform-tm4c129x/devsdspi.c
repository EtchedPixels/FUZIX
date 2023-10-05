#include <kernel.h>
#include <kdata.h>
#include <devsd.h>
#include <blkdev.h>
#include "inline-irq.h"
#include "tm4c129x.h"
#include "gpio.h"
#include "ssi.h"

/* TODO - shoul;d set a slow clock initially for old MMC cards */
void sd_spi_clock(bool go_fast)
{
}

void sd_spi_raise_cs(void)
{
  irqflags_t fl = __hard_di();

  gpio_write(GPIO_PORT('Q'), 1, 1);
#ifdef CONFIG_EK
  gpio_write(GPIO_PORT('Q'), 3, 1);
#else
  gpio_write(GPIO_PORT('H'), 4, 1);
#endif
  __hard_irqrestore(fl);
}

void sd_spi_lower_cs(void)
{
  irqflags_t fl = __hard_di();

  gpio_write(GPIO_PORT('Q'), 1, 1);
#ifdef CONFIG_EK
  gpio_write(GPIO_PORT('Q'), 3 , 0);
#else
  gpio_write(GPIO_PORT('H'), 4, 0);
#endif
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
