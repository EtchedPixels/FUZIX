#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include "mini_ide.h"

#define MAX_HD		2

uint8_t ide_present = 0;

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

/* Assembler glue */

extern void devide_read_data(uint8_t *p);
extern void devide_write_data(uint8_t *p);
extern uint8_t idepage;

/* FIXME: switch to the correct mpi slot on entry */
static int ide_transfer(uint_fast8_t minor, bool is_read, uint_fast8_t rawflag)
{
    uint16_t nb = 0;
    uint8_t *dptr;

    if (rawflag == 1 && d_blkoff(9))
         return -1;

    idepage = rawflag;
    dptr = udata.u_dptr;
    
    while(*status & 0x80);	/* Wait !BUSY */
    *devh = (minor & 0x80) ? 0xF0 : 0xE0 ;	/* LBA, device */
    while(*status & 0x80);	/* Wait !BUSY */
    *cylh = minor & 0x7F;	/* Slice number */

    while(udata.u_nblock--) {
        /* FIXME - slices of about 4MB might be saner! */
        *cyll = udata.u_block >> 8;	/* Each slice is 32MB */
        *sec = udata.u_block & 0xFF;
        *count = 1;
        while(!(*status & 0x40));	/* Wait DRDY */
        *cmd = is_read ? 0x20 : 0x30;
        while(!(*status & 0x08));	/* Wait DRQ */
        if (is_read)
          devide_read_data(dptr);
        else
          devide_write_data(dptr);
        dptr += 512;
        udata.u_block++;

        while(*status & 0x80);	/* Wait !BUSY */
        if (*status & 0x01) {	/* Error */
          kprintf("ide%d: error %x\n", *error);
          udata.u_error = EIO;
          return -1;
        }
        nb++;
    }
    return nb << BLKSHIFT;

}

int ide_open(uint_fast8_t minor, uint16_t flag)
{
    if (!(ide_present & (1 << (minor >> 7)))) {
        udata.u_error = ENODEV;
        return -1;
    }
    return 0;
}

int ide_read(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag)
{
    return ide_transfer(minor, true, rawflag);
}

int ide_write(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag)
{
    return ide_transfer(minor, false, rawflag);
}

int ide_ioctl(uint_fast8_t minor, uarg_t request, char *unused)
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
