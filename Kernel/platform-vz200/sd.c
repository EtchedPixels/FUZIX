#include <kernel.h>
#include <blkdev.h>
#include <devsd.h>

#ifdef CONFIG_SD

__sfr __at 56 spiconf;
__sfr __at 57 spidata;

static uint8_t spicf = 0xFF;

void sd_spi_raise_cs(void)
{
  spicf |= 2;
  spiconf = spicf;
}

void sd_spi_lower_cs(void)
{
  spicf &= 0xFD;
  spiconf = spicf;
}

void sd_spi_transmit_byte(uint8_t v) __z88dk_fastcall
{
  spidata = v;
}

uint8_t sd_spi_receive_byte(void)
{
  return spidata;
}

void sd_spi_clock(bool go_fast) __z88dk_fastcall
{
  spicf &= 0xFE;
  spicf |= go_fast;	/* Check which is fast */
}

COMMON_MEMORY

bool sd_spi_receive_sector(void) __naked
{
  __asm
    ld a, (_sd_raw)
    or a
    jr nz, from_user
    call map_buffers
    jr doread
to_user:
    call map_process_always
doread:
    ld bc, #57
    ld d,#255
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

bool sd_spi_transmit_sector(void) __naked
{
  __asm
    ld a, (_sd_raw)
    or a
    jr nz, from_user
    call map_buffers
    jr dowrite
from_user:
    call map_process_always
dowrite:
    ld bc, #57
writel:
    outi
    nop				; Check delay needed
    nop
    nop
    inc b
    outi
    nop
    nop
    nop
    jr nz, writel
    jp map_kernel_restore
  __endasm;
}

#endif
