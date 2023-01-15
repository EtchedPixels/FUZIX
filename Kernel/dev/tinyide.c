#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <tinyide.h>
#include "plt_ide.h"

uint8_t ide_present = 0;
uint8_t ide_master = 0xFF;
uint8_t ide_slave = 0xFF;

/* Assembler glue */
extern void devide_read_data(uint8_t *p);
extern void devide_write_data(uint8_t *p);
extern uint8_t idepage;

int ide_xfer(uint_fast8_t dev, bool is_read, uint32_t lba, uint8_t *dptr)
{
    while(*status & 0x80);	/* Wait !BUSY */
    *devh = (dev == ide_slave) ? 0xF0 : 0xE0 ;	/* LBA, device */
    while(*status & 0x80);	/* Wait !BUSY */

    if (lba < 32 && !is_read)
      panic("badlba");

    /* FIXME upper 4 bits */
    *cylh = lba >> 16;
    *cyll = lba >> 8;
    *sec = lba;
    *count = 1;
    while(!(*status & 0x40));	/* Wait DRDY */
    *cmd = is_read ? 0x20 : 0x30;
    while(!(*status & 0x08));	/* Wait DRQ */
    if (is_read)
      devide_read_data(dptr);
    else
      devide_write_data(dptr);

    while(*status & 0x80);	/* Wait !BUSY */
    if (*status & 0x01)		/* Error */
      return 0;
    return 1;
}

/* TODO - hook up ioctl - needs tinydisk tweaking */
#if 0
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
#endif
