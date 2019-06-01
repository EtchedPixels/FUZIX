#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include "mini_ide.h"

#define MAX_HD		2

uint8_t ide_present = 1;

#define data	((volatile uint8_t *)0xFF50)
#define error	((volatile uint8_t *)0xFF51)
#define count	((volatile uint8_t *)0xFF52)
#define sec	((volatile uint8_t *)0xFF53)
#define cyll	((volatile uint8_t *)0xFF54)
#define cylh	((volatile uint8_t *)0xFF55)
#define devh	((volatile uint8_t *)0xFF56)
#define status	((volatile uint8_t *)0xFF57)
#define cmd	((volatile uint8_t *)0xFF57)
#define datal	((volatile uint8_t *)0xFF58)

static int ide_transfer(uint8_t minor, bool is_read, uint8_t rawflag)
{
    uint16_t nb = udata.u_nblock;
    uint8_t *dptr = udata.u_dptr;

    if (rawflag == 1 && d_blkoff(9))
         return -1;
    
    while(*status & 0x80);	/* Wait !BUSY */
    *devh = (minor & 0x80) ? 0x50 : 0x40 ;	/* LBA, device */
    while(*status & 0x80);	/* Wait !BUSY */
    /* FIXME - slices of about 4MB might be saner! */
    *cylh = minor & 0x7F;	/* Slice number */
    *cyll = udata.u_block >> 8;	/* Each slice is 32MB */
    *sec = udata.u_block & 0xFF;
    *count = udata.u_nblock;
    while(!(*status & 0x40));	/* Wait DRDY */
    *cmd = is_read ? 0x20 : 0x30;
    while(*status & 0x08);	/* Wait DRQ */
    while(udata.u_nblock--) {
        unsigned int i;
        while(!(*status & 0x08));	/* Wait DRQ */
        if (is_read) {
            for (i = 0; i < 256; i++) {
                *dptr++ = *data;
                *dptr++ = *datal;
            }
        } else {
            for (i = 0; i < 256; i++) {
                *datal = dptr[1];
                *data = *dptr++;
                dptr++;
            }
        }
    }
    while(*status & 0x80);	/* Wait !BUSY */
    if (*status & 0x01) {	/* Error */
        kprintf("ide%d: error %x\n", *error);
        udata.u_error = EIO;
        return -1;
    }
    return nb;

}

int ide_open(uint8_t minor, uint16_t flag)
{
    if(minor > 1 || !(ide_present & (1 << minor))) {
        udata.u_error = ENODEV;
        return -1;
    }
    return 0;
}

int ide_read(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
    return ide_transfer(minor, true, rawflag);
}

int ide_write(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
    return ide_transfer(minor, false, rawflag);
}

int ide_ioctl(uint8_t minor, uarg_t request, char *unused)
{
    if (request != BLKFLSBUF)
        return -1;
    while(*status & 0x80);	/* Wait !BUSY */
    *devh = (minor & 0x80) ? 0x10 : 0x40 ;	/* LBA, device */
    while(*status & 0x80);	/* Wait !BUSY */
    while(!(*status & 0x40));	/* Wait DRDY */
    *cmd = 0xE7;
    while(*status & 0x80);	/* Wait !BUSY */
    return 0;
}

/* TODO: probe the devices more carefully and do EDD and LBA checks in discard
   code */
