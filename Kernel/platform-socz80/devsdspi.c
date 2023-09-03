/*
 *	SPI glue for the SocZ80. Based on Will Sowerbutts's UZI180 for SocZ80
 *	and his implementation for the N8VEM Mark 4
 */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <timer.h>
#include <stdbool.h>
#include "config.h"
#include <blkdev.h>

/* We should revisit this if we are willing to rely on the later VHDL
   being used. At that point we've got mode setting but more importantly
   the tx port is r/w as rx/tx so the port can sit in (c) */

__sfr __at 0x30 sd_spi_chipselect;
__sfr __at 0x31 sd_spi_status;
__sfr __at 0x32 sd_spi_tx;
__sfr __at 0x33 sd_spi_rx;
__sfr __at 0x34 sd_spi_divisor;
__sfr __at 0x35 sd_spi_gpio;		/* only on later VHDL */
__sfr __at 0x36 sd_spi_mode;		/* only on later VHDL */

#define SD_SPI_TX 0x32
#define SD_SPI_RX 0x33

void sd_spi_mode0(void)
{
  sd_spi_mode = 0;
}

void sd_spi_clock(bool go_fast)
{
//  sd_spi_mode0();
  /* Currently the sd driver just uses 'slow' and 'fast'. That's ok for
     sd but mmc really needs to be 16MHz */
  if (go_fast)
    sd_spi_divisor = 3;	//2
  else
    sd_spi_divisor = 255;
}

void sd_spi_raise_cs(void)
{
  sd_spi_chipselect = 0xFF;
}

void sd_spi_lower_cs(void)
{
  sd_spi_chipselect = 0xFE;
}

void sd_spi_transmit_byte(unsigned char byte)
{
  sd_spi_tx = byte;
}

uint8_t sd_spi_receive_byte(void)
{
  uint8_t r;
  sd_spi_tx = 0xFF;
  r = sd_spi_rx;
  return r;
}

COMMON_MEMORY

bool sd_spi_receive_sector(void) __naked
{
  __asm
    ld a, (_blk_op+BLKPARAM_IS_USER_OFFSET);
    ld hl, (_blk_op+BLKPARAM_ADDR_OFFSET);
    or a	; Set the Z flag up and save it, dont do it twice
    push af
    call nz,map_proc_always
    call rx256
    call rx256
xferout:
    pop af
    call nz,map_kernel
    ret
rx256:
    ld a,#0xFF
    ld bc, #SD_SPI_RX	 ; b = 0, c = port
rx256_1:
    out (SD_SPI_TX),a   ; we could use (c),a on newer VHDL
    ini
    jr nz, rx256_1
    ret
  __endasm;
}

bool sd_spi_transmit_sector(void) __naked
{
  __asm
    ld a, (_blk_op+BLKPARAM_IS_USER_OFFSET)
    ld hl, (_blk_op+BLKPARAM_ADDR_OFFSET)
    or a	; Set the Z flag up and save it, dont do it twice
    push af
    call nz,map_proc_always
    ld a,#0xFF
    ld bc, #SD_SPI_TX
    otir
    otir
    jr xferout
  __endasm;
}
