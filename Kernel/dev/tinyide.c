#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <tinyide.h>
#include "plt_ide.h"

#ifdef CONFIG_TD_IDE

uint8_t ide_present = 0;
uint8_t ide_dev[TD_IDE_NUM];
uint8_t ide_unit;

int ide_xfer(uint_fast8_t dev, bool is_read, uint32_t lba, uint8_t *dptr)
{
    ide_unit = ide_dev[dev];
    while(ide_read(status) & 0x80);	/* Wait !BUSY */
    ide_write(devh, (ide_unit & 1) ? 0xF0 : 0xE0) ;	/* LBA, device */
    while(ide_read(status) & 0x80);	/* Wait !BUSY */

    if (lba < 32 && !is_read)
      panic("badlba");

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

/* TODO - hook up ioctl - needs tinydisk tweaking */
#if 0
int ide_ioctl(uint_fast8_t minor, uarg_t request, char *unused)
{
    if (request != BLKFLSBUF)
        return -1;
    ide_unit = ide_unit[minor];
    while(ide_read(status) & 0x80);	/* Wait !BUSY */
    ide_write(devh , (minor & 0x80) ? 0x10 : 0x40) ;	/* LBA, device */
    while(ide_read(status) & 0x80);	/* Wait !BUSY */
    while(!(ide_read(status) & 0x40));	/* Wait DRDY */
    ide_write(cmd, 0xE7);
    while(ide_read(status) & 0x80);	/* Wait !BUSY */
    return 0;
}
#endif

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
            call map_process_always  	            ; map user memory first if required
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
            call map_process_always                 ; else map user memory first if required
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
