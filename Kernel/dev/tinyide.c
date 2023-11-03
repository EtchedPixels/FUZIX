#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <tinyide.h>
#include "plt_ide.h"

#ifdef CONFIG_TD_IDE

uint8_t ide_present = 0;
uint8_t ide_unit;

int ide_xfer(uint_fast8_t dev, bool is_read, uint32_t lba, uint8_t *dptr)
{
    ide_unit = dev;
    while(ide_read(status) & 0x80);	/* Wait !BUSY */
    ide_write(devh, (ide_unit & 1) ? 0xF0 : 0xE0) ;	/* LBA, device */
    while(ide_read(status) & 0x80);	/* Wait !BUSY */

    /* FIXME upper 4 bits */
    ide_write(cylh, lba >> 16);
    ide_write(cyll, lba >> 8);
    ide_write(sec, lba);
    ide_write(count, 1);
    while(!(ide_read(status) & 0x40));	/* Wait DRDY */
    ide_write(cmd, is_read ? 0x20 : 0x30);
    while(!(ide_read(status) & 0x08));	/* Wait DRQ */
    if (is_read)
      devide_read_data(dptr);
    else
      devide_write_data(dptr);

    while(ide_read(status) & 0x80);	/* Wait !BUSY */
    if (ide_read(status) & 0x01)	/* Error */
      return 0;
    return 1;
}

#ifdef CONFIG_TD_IDE_CHS

uint8_t ide_spt[TD_IDE_NUM];
uint8_t ide_heads[TD_IDE_NUM];
uint16_t ide_cyls[TD_IDE_NUM];

static uint32_t last_lba_b;
static uint8_t last_dev = 0xFF;
static uint8_t last_head;
static uint16_t last_cyl;

int ide_chs_xfer(uint_fast8_t drive, bool is_read, uint32_t lba, uint8_t *dptr)
{
    uint32_t sector;
    uint32_t head;
    uint16_t cyl;

    ide_unit = drive;

    /* Avoid the expensive 32bit maths (on 8bit anyway) by spotting
       further requests on the same head/cylinder as for those we just
       need to adjust the sector register */
    if (drive == last_dev && lba >= last_lba_b &&
      lba < last_lba_b + ide_spt[drive]) {
        head = last_head;
        cyl = last_cyl;
        sector = lba - last_lba_b;
    } else {
      /* Didn't manage to shortcut the maths */
      head = lba / ide_spt[drive];
      cyl = head / ide_heads[drive];
      sector = lba % ide_spt[drive];
      head %= ide_heads[drive];
      head |= 0xA0;
      if (drive & 1)
        head |= 0x10;
      last_dev = drive;
      last_lba_b = lba - sector;
      last_head = head;
      last_cyl = cyl;
    }

    while(ide_read(status) & 0x80);	/* Wait !BUSY */
    ide_write(devh, head) ;		/* head / device */
    while(ide_read(status) & 0x80);	/* Wait !BUSY */

    ide_write(cylh, cyl >> 8);
    ide_write(cyll, cyl);
    ide_write(sec, sector + 1);		/* 1 based */
    ide_write(count, 1);
    while(!(ide_read(status) & 0x40));	/* Wait DRDY */
    ide_write(cmd, is_read ? 0x20 : 0x30);
    while(!(ide_read(status) & 0x08));	/* Wait DRQ */
    if (is_read)
      devide_read_data(dptr);
    else
      devide_write_data(dptr);

    while(ide_read(status) & 0x80);	/* Wait !BUSY */
    if (ide_read(status) & 0x01)	/* Error */
      return 0;
    return 1;
}
#endif

int ide_ioctl(uint_fast8_t dev, uarg_t request, char *unused)
{
    if (request != BLKFLSBUF)
        return -1;
    ide_unit = dev;
    while(ide_read(status) & 0x80);	/* Wait !BUSY */
    ide_write(devh , (dev & 0x01) ? 0xF0 : 0xE0) ;	/* LBA, device */
    while(ide_read(status) & 0x80);	/* Wait !BUSY */
    while(!(ide_read(status) & 0x40));	/* Wait DRDY */
    ide_write(cmd, 0xE7);
    while(ide_read(status) & 0x80);	/* Wait !BUSY */
    return 0;
}

#ifndef IDE_NONSTANDARD_XFER
#ifdef CONFIG_TINYIDE_SDCCPIO
/* Port I/O: Currently Z80/Z180 only */

COMMON_MEMORY

void devide_read_data(uint8_t *p) __naked
{
    __asm
            ld a, (_td_raw) 			    ; user/kernel/swap
            pop de
            pop hl				    ; HL is now the data
            push hl
            push de
            ld bc, #IDE_REG_DATA                    ; setup port number
                                                    ; and count
#ifdef SWAPDEV
	    cp #2
            jr nz, not_swapin
            ld a, (_td_page)			    ; swap page to map
            call map_for_swap
            jr doread
not_swapin:
#endif
            or a                                    ; test is_user
            jr z, rd_kernel
            call map_proc_always  	            ; map user memory first if required
            jr doread
rd_kernel:
            call map_buffers
doread:
            inir                                    ; transfer first 256 bytes
            inir                                    ; transfer second 256 bytes
            jp map_kernel_restore                   ; else map kernel then return
    __endasm;
}

void devide_write_data(uint8_t *p) __naked
{
    __asm
            ld a, (_td_raw) 			    ; user/kernel/swap
            pop de
            pop hl				    ; HL is now the data
            push hl
            push de
            ld bc, #IDE_REG_DATA                    ; setup port number
                                                    ; and count
#ifdef SWAPDEV
	    cp #2
            jr nz, not_swapout
            ld a, (_td_page)			    ; swap page to map
            call map_for_swap
            jr dowrite
not_swapout:
#endif
            or a                                    ; test is_user
            jr z, wr_kernel
            call map_proc_always                 ; else map user memory first if required
            jr dowrite
wr_kernel:
            call map_buffers
dowrite:
            otir                                    ; transfer first 256 bytes
            otir                                    ; transfer second 256 bytes
            jp map_kernel_restore                   ; else map kernel then return
    __endasm;
}

#endif
#endif
#endif
