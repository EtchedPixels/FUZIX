/*-----------------------------------------------------------------------*/
/* MegaSD driver for MegaFlashROM SCC+ SD                                */
/*-----------------------------------------------------------------------*/

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <timer.h>
#include <stdbool.h>
#include "config.h"
#include <blkdev.h>
#include <devsd.h>
#include "msx2.h"

/*
 * MegaFlashRom SCC+ SD contains an slot expander with several devices.
 * The MegaSD interface is in subslot 3 and it contains an ASCII 8Kb mapper
 * with the sdcard io registers mapped into page 0x40
 */

#define MSD_MAGIC_ADDR 0x4110
#define MSD_MAGIC_PAGE 0x0e

#define MSD_PAGE      0x40
#define MSD_SUBSLOT   0x3

#define MFR_BANKSEL0  0x6000
#define MFR_BANKSEL1  0x6800

/* bank 0 is in 0x4000-0x5fff and the sd interface mapped io */

#define MSD_RDWR      0x4000
#define MSD_CS        0x5000
#define MSD_DEVSEL    0x5800

#define readb(x)            *((volatile uint8_t *)x)
#define writeb(val,x)       *((volatile uint8_t *)x) = val

#ifdef DEVICE_SD


/* slot and subslot containing the sd interface */
uint8_t slotmfr;

int megasd_probe()
{
    uint8_t *sigp = (uint8_t *) MSD_MAGIC_ADDR;
    uint8_t slot = 1;

    for (slot = 1; slot < 3; slot++) {
        /* try to find MegaFlashRom signature in slots 1 and 2 */
        slotmfr = 0x80 | MSD_SUBSLOT << 2 | slot;
        mapslot_bank1(slotmfr);
        writeb(MSD_MAGIC_PAGE, MFR_BANKSEL0);
        if (sigp[0] == 'M' && sigp[1] == 'e' && sigp[2] == 'g' && sigp[3] == 'a')
            goto found;
    }
    mapslot_bank1(slotram);
    return 0;

found:
    mapslot_bank1(slotram);
    kprintf("MegaSD found in slot %d-3\n", slot);
    return 1;
}

void sd_spi_map_interface()
{
    mapslot_bank1(slotmfr);
    writeb(MSD_PAGE, MFR_BANKSEL0);
}

void sd_spi_unmap_interface()
{
    mapslot_bank1(slotram);
}

void sd_spi_clock(bool go_fast)
{
    go_fast;
}

void sd_spi_raise_cs(void)
{
    sd_spi_map_interface();
    writeb(sd_drive, MSD_DEVSEL);

    /* reading from MSD_CS raises CS for all cards */

    readb(MSD_CS);

    sd_spi_unmap_interface();
}

void sd_spi_lower_cs(void)
{
    /* happens automatically when sending */
}

void sd_spi_transmit_byte(unsigned char byte)
{
    sd_spi_map_interface();

    writeb(byte, MSD_RDWR);

    sd_spi_unmap_interface();
}

uint8_t sd_spi_receive_byte(void)
{
    unsigned char c;

    sd_spi_map_interface();

    c = readb(MSD_RDWR);

    sd_spi_unmap_interface();

    return c;
}

/*
 * Block transfer is now equivalent to memory copy from MegaSD mapped i/o to a ram page.
 * Target page is always mapped to slot_page2, and the target address offset accordingly.
 *
 */
bool sd_spi_receive_sector(void) __naked
{
    __asm

    ; map sd interface
    ;
    ld a,(_slotmfr)
    call _mapslot_bank1
    ld a, #MSD_PAGE
    ld (MFR_BANKSEL0),a

    ld a, (_blk_op+BLKPARAM_IS_USER_OFFSET)
    ld de, (_blk_op+BLKPARAM_ADDR_OFFSET)
    push af
    or a
    jr z, starttx

    ; map process target page in slot_page2 if needed
    ;
    ld a,d
    and #0xC0
    rlca
    rlca    ;  a contains the page to map
    ld b,#0
    ld c,a
    ld hl,#U_DATA__U_PAGE
    add hl,bc
    ld a,(hl)
    out(_RAM_PAGE2),a

starttx:
    ; calculate offset address in target page
    ld a,d
    and #0x3F
    or #0x80
    ld d,a
    ld hl,#MSD_RDWR
    ld bc,#512
    jp looptxrx
    __endasm;
}

bool sd_spi_transmit_sector(void) __naked
{
    __asm

    ; map sd interface
    ;
    ld a,(_slotmfr)
    call _mapslot_bank1
    ld a, #MSD_PAGE
    ld (MFR_BANKSEL0),a

    ; map process target page in slot_page2
    ;
    ld a, (_blk_op+BLKPARAM_IS_USER_OFFSET)
    ld de, (_blk_op+BLKPARAM_ADDR_OFFSET);
    push af
    or a
    jr z, startrx

    ; map process target page in slot_page2 if needed
    ;
    ld a,d
    and #0xC0
    rlca
    rlca    ;  a contains the page to map
    ld b,#0
    ld c,a
    ld hl,#U_DATA__U_PAGE
    add hl,bc
    ld a,(hl)
    out(_RAM_PAGE2),a

startrx:
    ; calculate offset address in target page
    ld a,d
    and #0x3F
    or #0x80
    ld d,a
    ld hl,#MSD_RDWR
    ex de,hl
    ld bc,#512
looptxrx:
    ldi	; 16x ldi: 19% faster
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    jp pe, looptxrx

    ; unmap interface
    ;
    ld a,(_slotram)
    call _mapslot_bank1
    pop af
    or a
    ret z
    jp _map_kernel
    __endasm;
}

#endif
