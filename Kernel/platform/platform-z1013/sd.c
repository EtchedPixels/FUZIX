#include <kernel.h>
#include <blkdev.h>
#include <tinysd.h>
#include <z80softspi.h>

#ifdef CONFIG_TD_SD

/*
 *	SD card bit bang. For now just a single card to get us going. We
 *	should fix the cs asm to allow for multiple cards
 *
 *	Bits
 *	7: MISO
 *	6: CLK
 *	5: \CS card 0
 *	4: MOSI
 */

__sfr __at 0x02 pio_c;
__sfr __at 0x00 pio_d;

void sd_setup(void)
{
    spi_piostate = 0xE0;

    spi_data = 0x10;
    spi_clock = 0x40;
}

void sd_spi_raise_cs(void)
{
    spi_piostate |= 0x08;
    pio_d = spi_piostate;
}

void sd_spi_lower_cs(void)
{
    spi_piostate &= ~0x08;
    pio_d = spi_piostate;
}

void sd_spi_slow(void)
{
}

void sd_spi_fast(void)
{
}

COMMON_MEMORY

bool sd_spi_receive_sector(uint8_t *p) __naked
{
  __asm
    pop de
    pop hl
    push hl
    push de
    ld a, (_td_raw)
    or a
    jr nz, from_user
    call map_buffers
    jr doread
to_user:
    call map_proc_always
doread:
    call _sd_spi_rx_sector
    jp map_kernel_restore
  __endasm;
}

bool sd_spi_transmit_sector(uint8_t *p) __naked
{
  __asm
    pop de
    pop hl
    push hl
    push de
    ld a, (_td_raw)
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
