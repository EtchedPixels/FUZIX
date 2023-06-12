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

#ifdef CONFIG_SD


/* slot and subslot containing the sd interface */
static uint8_t slotmfr;

int megasd_probe(void)
{
    uint8_t *sigp = (uint8_t *) MSD_MAGIC_ADDR;
    uint8_t slot;
    irqflags_t irq = di();

    for (slot = 1; slot < 3; slot++) {
        /* try to find MegaFlashRom signature in slots 1 and 2 */
        slotmfr = 0x80 | MSD_SUBSLOT << 2 | slot;
        mapslot_bank1(slotmfr);
        writeb(MSD_MAGIC_PAGE, MFR_BANKSEL0);
        if (sigp[0] == 'M' && sigp[1] == 'e' && sigp[2] == 'g' && sigp[3] == 'a')
            goto found;
    }
    mapslot_bank1(slotram);
    irqrestore(irq);
    return 0;

found:
    mapslot_bank1(slotram);
    irqrestore(irq);
    kprintf("MegaSD found in slot %d-3\n", slot);
    return 1;
}

static irqflags_t sd_spi_map_interface(void)
{
    mapslot_bank1(slotmfr);
    writeb(MSD_PAGE, MFR_BANKSEL0);
    return di();
}

static void sd_spi_unmap_interface(irqflags_t irq)
{
    mapslot_bank1(slotram);
    irqrestore(irq);
}

void sd_spi_clock(bool go_fast)
{
    go_fast;
}

void sd_spi_raise_cs(void)
{
    irqflags_t irq = sd_spi_map_interface();
    writeb(sd_drive, MSD_DEVSEL);

    /* reading from MSD_CS raises CS for all cards */

    readb(MSD_CS);

    sd_spi_unmap_interface(irq);
}

void sd_spi_lower_cs(void)
{
    /* happens automatically when sending */
}

void sd_spi_transmit_byte(unsigned char byte)
{
    irqflags_t irq = sd_spi_map_interface();

    writeb(byte, MSD_RDWR);

    sd_spi_unmap_interface(irq);
}

uint8_t sd_spi_receive_byte(void)
{
    unsigned char c;
    irqflags_t irq = sd_spi_map_interface();

    c = readb(MSD_RDWR);

    sd_spi_unmap_interface(irq);

    return c;
}

/*
 * Block transfer is now equivalent to memory copy from MegaSD mapped i/o to a ram page.
 * Target page is always mapped to slot_page2, and the target address offset accordingly.
 *
 */
static void sd_spi_txrx_sector(bool is_read)
{
    uint16_t addr, len, len2;
    uint8_t *page;
    uint8_t page_offset;
    irqflags_t irq;

    addr = ((uint16_t) blk_op.addr) % 0x4000 + 0x8000;
    page_offset = (((uint16_t)blk_op.addr) / 0x4000);
    page = &udata.u_page;

    len = 0xC000 - addr;
    irq = sd_spi_map_interface();

    if (blk_op.is_user) {
        RAM_PAGE2 = *(page + page_offset);
        if (is_read)
                memcpy((uint8_t *)addr, (uint8_t *)MSD_RDWR,
                        len < BLKSIZE ? len : BLKSIZE);
        else
                memcpy((uint8_t *)MSD_RDWR, (uint8_t *)addr,
                        len < BLKSIZE ? len : BLKSIZE);
        if (len < BLKSIZE) {
                /* handle page crossing */
                len2 = BLKSIZE - len;
                RAM_PAGE2 = *(page + page_offset + 1);
                if (is_read)
                        memcpy((uint8_t *)0x8000, (uint8_t *)MSD_RDWR, len2);
                else
                        memcpy((uint8_t *)MSD_RDWR, (uint8_t *)0x8000, len2);
        }
        RAM_PAGE2 = 1;
    } else {
        /* kernel only */
        if (is_read)
                memcpy(blk_op.addr, (uint8_t *)MSD_RDWR, BLKSIZE);
        else
                memcpy((uint8_t *)MSD_RDWR, blk_op.addr, BLKSIZE);
    }
    sd_spi_unmap_interface(irq);
}

bool sd_spi_receive_sector(void)
{
    sd_spi_txrx_sector(true);
    return true;
}

bool sd_spi_transmit_sector(void)
{
    sd_spi_txrx_sector(false);
    return true;
}

#endif
