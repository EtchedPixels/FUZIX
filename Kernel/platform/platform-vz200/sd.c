#include <kernel.h>
#include <blkdev.h>
#include <tinysd.h>
#include "printf.h"

__sfr __at 56 spiconf;
__sfr __at 57 spidata;

static uint8_t spicf = 0xFF;

void sd_spi_raise_cs(void)
{
  spicf &= ~2;
  spiconf = spicf;
}

void sd_spi_lower_cs(void)
{
  spicf |= 2;
  spiconf = spicf;
}

void sd_spi_tx_byte(uint8_t v)
{
  spidata = v;
}

uint8_t sd_spi_rx_byte(void)
{
  uint8_t v;
  sd_spi_tx_byte(0xFF);
  return spidata;
}

/* TODO call this appropriately after boot */
void sd_spi_clock(bool go_fast)
{
  spicf &= 0xFE;
  spicf |= go_fast;
}

COMMON_MEMORY

bool sd_spi_rx_sector(uint8_t *ptr) __naked
{
  __asm
    pop bc
    pop	de
    pop hl
    push hl
    push de
    push bc
    ld a, (_td_raw)
    or a
    jr nz, to_user
    call map_buffers
    jr doread
to_user:
    call map_proc_always
doread:
    ld bc, #57
    ld d,#0xFF
readl:
    out (c),d
    nop				; Check delay needed
    ini
    inc b
    out (c),d
    nop
    ini
    jr nz, readl
    jp map_kernel_restore
  __endasm;
}

bool sd_spi_tx_sector(uint8_t *ptr) __naked
{
  __asm
    pop bc
    pop	de
    pop hl
    push hl
    push de
    push bc
    ld a, (_td_raw)
    or a
    jr nz, from_user
    call map_buffers
    jr dowrite
from_user:
    call map_proc_always
dowrite:
    ld bc, #57
writel:
    outi
    nop				; Check delay needed
    inc b
    outi
    nop
    jr nz, writel
    jp map_kernel_restore
  __endasm;
}
