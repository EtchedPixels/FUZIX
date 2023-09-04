#include <kernel.h>
#include <tinysd.h>
#include "printf.h"

__sfr __at 0xE7 divmmc_cs;
__sfr __at 0xEB divmmc_data;

void sd_spi_raise_cs(void)
{
    divmmc_cs = 0xFF;//0x03;		/* Active low */
}

void sd_spi_transmit_byte(uint8_t b) SD_SPI_CALLTYPE
{
    divmmc_data = b;
}

uint8_t sd_spi_receive_byte(void)
{
    return divmmc_data;
}

/*
 *	The ZX Uno works on the bits, Zesarux for some reason
 *	just does "FE is card 0 else card 1", meaning you can't unselect
 *	and it mishandles most values.
 */
void sd_spi_lower_cs(void)
{
    if (tinysd_unit == 0)
      divmmc_cs = 0xFE;	/* Lower bit 0 (active low) */
    else
      divmmc_cs = 0xFD;	/* Lower bit 1 (active low) */
}

void sd_spi_fast(void)
{
}

void sd_spi_slow(void)
{
}

COMMON_MEMORY

/*
 * FIXME: swap support
 *
 * Could also unroll these a bit for speed
 */

bool sd_spi_receive_sector(uint8_t *data) __naked SD_SPI_CALLTYPE
{
  __asm
#ifdef SD_SPI_BANKED
    pop bc
    pop de
    pop hl
    push hl
    push de
    push bc
#endif    
    ld a, (_td_raw)
    push af
#ifdef SWAPDEV
    cp #2
    jr nz, not_swapin
    ld a,(_td_page)
    call map_for_swap
    jr doread
not_swapin:
#endif
    or a
    call nz,map_proc_always
doread:
    ld bc, #0xEB	 ; b = 0, c = port
    ld a,#0x05
    out (0xfe),a
    inir
    ld a,#0x02
    out (0xfe),a
    inir
    ld a,(_vtborder)
    out (0xfe),a
    pop af
    or a
    jp nz,map_kernel
    ret
  __endasm;
}

bool sd_spi_transmit_sector(uint8_t *data) __naked SD_SPI_CALLTYPE
{
  __asm
#ifdef SD_SPI_BANKED
    pop bc
    pop de
    pop hl
    push hl
    push de
    push bc
#endif    
    ld a, (_td_raw)
    push af
#ifdef SWAPDEV
    cp #2
    jr nz, not_swapout
    ld a, (_td_page)
    call map_for_swap
    jr dowrite
not_swapout:
#endif
    or a
    call nz,map_proc_always
dowrite:
    ld bc, #0xEB
    ld a,#0x05
    out (0xfe),a
    otir
    ld a,#0x02
    out (0xfe),a
    otir
    ld a,(_vtborder)
    out (0xfe),a
    pop af
    or a
    jp nz,map_kernel
    ret
  __endasm;
}
