#include <kernel.h>
#include <tinydisk.h>
#include <tinysd.h>
#include <z80softspi.h>

#include "rcbus.h"

#ifdef CONFIG_TD_SD

/*
 *	SD card bit bang. For now just a single card to get us going. We
 *	should fix the cs asm to allow for multiple cards
 *
 *	Bits
 *	7: MISO
 *	4: CLK
 *	3: \CS card 0
 *	0: MOSI
 *
 *	TODO: we should allow for SD cards on the 0xC0 KIO if present on
 *	an extreme build ?
 */

uint16_t pio_c;

void pio_setup(void)
{
    spi_piostate = 0xE0;

    if (kio_port == 0x80) {
      pio_c = 0x83;
      spi_port = 0x82;
    } else if (eipc_present) {
      pio_c = 0x1C;
      spi_port = 0x1D;
    } else {
      pio_c = 0x6B;
      spi_port = 0x69;
    }
    /* Pin names in brackets are mapping onto Gluino */
    /* Data on PIOB bit 0 (D11 / CIPO */
    /* Clock on PIOB bit 4 (D13 / SCL) */
    /* CS on PIOB bit 3 (D10 / SS) */
    /* Data on PIOB bit 7 (D12 /COPI) */
    spi_data = 0x01;
    spi_clock = 0x10;

    out16(pio_c, 0xCF);		/* Mode 3 */
    out16(pio_c, 0xE6);		/* MISO input, unused as input (so high Z) */
    out16(pio_c, 0x07);		/* No interrupt, no mask */
}

void sd_spi_raise_cs(void)
{
    out16(spi_port, spi_piostate |= 0x08);
}

void sd_spi_lower_cs(void)
{
    spi_piostate &= ~0x08;
    out16(spi_port, spi_piostate);
}

void sd_spi_fast(void)
{
}

void sd_spi_slow(void)
{
}

COMMON_MEMORY

bool sd_spi_receive_sector(uint8_t *data) __naked
{
  __asm
    pop bc
    pop de
    pop hl
    push hl
    push de
    push bc
    ld a, (_td_raw)
#ifdef SWAPDEV
    cp #2
    jr nz, not_swapin
    ld a,(_td_page)
    call map_for_swap
    jr doread
not_swapin:
#endif
    or a
    jr nz, to_user
    call map_buffers
    jr doread
to_user:
    call map_proc_always
doread:
    call _sd_spi_rx_sector
    jp map_kernel_restore
  __endasm;
}

bool sd_spi_transmit_sector(uint8_t *data) __naked
{
  __asm
    pop bc
    pop de
    pop hl
    push hl
    push de
    push bc
    ld a, (_td_raw)
#ifdef SWAPDEV
    cp #2
    jr nz, not_swapout
    ld a, (_td_page)
    call map_for_swap
    jr dowrite
not_swapout:
#endif
    or a
    jr nz, from_user
    call map_buffers
    jr dowrite
from_user:
    call map_proc_always
dowrite:
    call _sd_spi_tx_sector
    jp map_kernel_restore
  __endasm;
}

#endif
