#include <kernel.h>
#include <tinysd.h>
#include <printf.h>

/* FIXME: optimise by unrolling inir and otir 16 ways */

__sfr __at 0x1F zxmmc_cs;
__sfr __at 0x3F zxmmc_data;

void sd_spi_raise_cs(void)
{
    zxmmc_cs = 0xF7;		/* Active low NMI off */
}

void sd_spi_tx_byte(uint8_t b) SD_SPI_CALLTYPE
{
    zxmmc_data = b;
}

uint8_t sd_spi_rx_byte(void)
{
    return zxmmc_data;
}

void sd_spi_lower_cs(void)
{
    if (tinysd_unit == 0)
      zxmmc_cs = 0xF6;	/* Lower bit 0 (active low) */
    else
      zxmmc_cs = 0xF5;	/* Lower bit 1 (active low) */
}

void sd_spi_fast(void)
{
}

void sd_spi_slow(void)
{
}

COMMON_MEMORY

/*
 * Could also unroll these a bit for speed
 */

bool sd_spi_rx_sector(uint8_t *data) __naked SD_SPI_CALLTYPE
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
    ld bc, #0x3f	 ; b = 0, c = port
    ld a,#0x05
    out (0xfe),a
    inir
    ld a,#0x02
    out (0xfe),a
    inir
    ld a,(_vtborder)
    out (0xfe),a
    in a,(0x3f)		; required according to CP/M driver
    nop
    nop
    in a,(0x3f)
    pop af
    or a
    jp nz,map_kernel
    ret
  __endasm;
}

bool sd_spi_tx_sector(uint8_t *data) __naked SD_SPI_CALLTYPE
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
    ld bc, #0x3f
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

