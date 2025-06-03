#include <kernel.h>
#include <blkdev.h>
#include <tinysd.h>
#include <z80softspi.h>

/*
 *	SD card bit bang. For now just a single card to get us going. We
 *	should fix the cs asm to allow for multiple cards
 *
 *	Bits
 *	0: MISO
 *	1: CLK
 *	2: MOSI
 *	3: \CS card 0
 *	4: \CS card 1
 *
 *	We don't use 5-7. We could put an interrupt on these, more CS lines
 *	or another else unrelated.
 */

/* PIO port B */
__sfr __at 0x19	piob_d;
__sfr __at 0x1b piob_c;

void pio_setup(void)
{
    spi_piostate = 0xE0;
    spi_port = 0x19;

    piob_c = 0xCF;		/* Mode 3 */
    piob_c = 0xE1;		/* MISO input, unused as input (so high Z) */
    /* No vector loading for now - might need if we want to support an SPI
       device with interrupts (eg ethernet) */
    piob_c = 0x07;		/* No interrupt, no mask */
}

void sd_spi_raise_cs(void)
{
    piob_d = spi_piostate |= 0x18;
}

void sd_spi_lower_cs(void)
{
    if (tinysd_unit == 0)
        spi_piostate &= ~0x08;
    else
        spi_piostate &= ~0x10;
    piob_d = spi_piostate;
}

void sd_spi_fast(void)
{
}

void sd_spi_slow(void)
{
}

COMMON_MEMORY

bool sd_spi_rx_sector(uint8_t *data) __naked __z88dk_fastcall
{
  __asm
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
    call _sd_spi_rx_sector
    pop af
    or a
    jp nz,map_kernel
    ret
  __endasm;
}

bool sd_spi_tx_sector(uint8_t *data) __naked __z88dk_fastcall
{
  __asm
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
    call nz,map_proc_always
dowrite:
    call _sd_spi_tx_sector
    pop af
    or a
    jp nz,map_kernel
    ret
  __endasm;
}

