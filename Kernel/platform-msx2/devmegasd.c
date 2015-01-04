/*-----------------------------------------------------------------------*/
/* MegaSD driver for MegaFlashROM SCC+ SD                                */
/*-----------------------------------------------------------------------*/

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <timer.h>
#include <stdbool.h>
#include "config.h"

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

extern int mapslot_bank1(uint8_t slot);
extern uint8_t slotram;

/* slot and subslot containing the sd interface */
uint8_t slotmfr;

int megasd_probe()
{
    uint8_t *sigp = (uint8_t *) MSD_MAGIC_ADDR;
    uint8_t slot = 1;

    kprintf("MegaSD...");

    for (slot = 1; slot < 3; slot++) {
        /* try to find MegaFlashRom signature in slots 1 and 2 */
        slotmfr = 0x80 | MSD_SUBSLOT << 2 | slot;
        mapslot_bank1(slotmfr);
        writeb(MSD_MAGIC_PAGE, MFR_BANKSEL0);
        if (sigp[0] == 'M' && sigp[1] == 'e' && sigp[2] == 'g' && sigp[3] == 'a')
            goto found;
    }
    mapslot_bank1(slotram);
    kprintf("not found\n");
    return 0;

found:
    mapslot_bank1(slotram);
    kprintf("found in slot %d-3\n", slot);
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

void sd_spi_clock(uint8_t drive, bool go_fast)
{
    drive; /* not used */
    go_fast;
}

void sd_spi_raise_cs(uint8_t drive)
{
    drive; /* not used */

    sd_spi_map_interface();
    writeb(drive, MSD_DEVSEL);

    /* reading from MSD_CS raises CS for all cards */

    readb(MSD_CS);

    sd_spi_unmap_interface();
}

void sd_spi_lower_cs(uint8_t drive)
{
    drive; /* not used */

    /* happens automatically when sending */
}

void sd_spi_transmit_byte(uint8_t drive, unsigned char byte)
{
    drive; /* not used */

    sd_spi_map_interface();

    writeb(byte, MSD_RDWR);

    sd_spi_unmap_interface();
}

uint8_t sd_spi_receive_byte(uint8_t drive)
{
    unsigned char c;
    drive; /* not used */

    sd_spi_map_interface();

    c = readb(MSD_RDWR);

    sd_spi_unmap_interface();

    return c;
}

bool sd_spi_receive_block(uint8_t drive, uint8_t *ptr, unsigned int length)
{
    drive; /* not used */

    sd_spi_map_interface();

    while(length--) *ptr++ = readb(MSD_RDWR);

    sd_spi_unmap_interface();

    return true;
}

bool sd_spi_transmit_block(uint8_t drive, uint8_t *ptr, int length)
{
    drive; /* not used */

    sd_spi_map_interface();

    while(length--) writeb(*(ptr++), MSD_RDWR);

    sd_spi_unmap_interface();

    return true;
}

#endif
