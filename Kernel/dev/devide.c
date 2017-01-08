/*-----------------------------------------------------------------------*/
/* IDE interface driver                                                  */
/* 2014-11-02 Will Sowerbutts (unreleased UZI-mark4)                     */
/* 2014-12-22 WRS ported to Fuzix                                        */
/* 2014-12-25 WRS updated to also support P112 GIDE                      */
/* 2015-01-04 WRS updated to new blkdev API                              */
/* 2015-01-25 WRS updated to newer blkdev API                            */
/*-----------------------------------------------------------------------*/

#define _IDE_PRIVATE

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <timer.h>
#include <devide.h>
#include <blkdev.h>

bool devide_wait(uint8_t bits)
{
    uint8_t status;
    timer_t timeout;

    timeout = set_timer_sec(20);

    while(true){
        status = devide_readb(ide_reg_status);

        if((status & (IDE_STATUS_BUSY | IDE_STATUS_ERROR | bits)) == bits)
            return true;

        if(((status & (IDE_STATUS_BUSY | IDE_STATUS_ERROR)) == IDE_STATUS_ERROR) || /* error */
           (status == 0x00) || /* zeta-v2 PPIDE: status=0x00 indicates no slave drive present */
           (status == 0xFF) || /* zeta-v2 PPIDE: status=0xFF indicates neither master nor slave drive present */
           (status == 0x87)){  /* n8vem-mark4:   status=0x87 indicates neither master nor slave drive present */
            kprintf("ide error, status=%x\n", status);
            return false;
        }

        if(timer_expired(timeout)){
            kprintf("ide timeout, status=%x\n", status);
            return false;
        }
    }
}

uint8_t devide_transfer_sector(void)
{
    uint8_t drive;
#if defined(__SDCC_z80) || defined(__SDCC_z180) || defined(__SDCC_gbz80) || defined(__SDCC_r2k) || defined(__SDCC_r3k)
    uint8_t *p;
#endif


    drive = blk_op.blkdev->driver_data & DRIVE_NR_MASK;

    ide_select(drive);

#if defined(__SDCC_z80) || defined(__SDCC_z180) || defined(__SDCC_gbz80) || defined(__SDCC_r2k) || defined(__SDCC_r3k)
    /* sdcc sadly unable to figure this out for itself yet */
    p = ((uint8_t *)&blk_op.lba)+3;
    devide_writeb(ide_reg_lba_3, (*(p--) & 0x0F) | ((drive == 0) ? 0xE0 : 0xF0)); // select drive, start loading LBA
    devide_writeb(ide_reg_lba_2, *(p--));
    devide_writeb(ide_reg_lba_1, *(p--));
    devide_writeb(ide_reg_lba_0, *p);
#else
    devide_writeb(ide_reg_lba_3, ((blk_op.lba >> 24) & 0xF) | ((drive == 0) ? 0xE0 : 0xF0)); // select drive, start loading LBA
    devide_writeb(ide_reg_lba_2, (blk_op.lba >> 16));
    devide_writeb(ide_reg_lba_1, (blk_op.lba >> 8));
    devide_writeb(ide_reg_lba_0, blk_op.lba);
#endif

    if (!devide_wait(IDE_STATUS_READY))
	goto fail;

    devide_writeb(ide_reg_sec_count, 1);
    devide_writeb(ide_reg_command, blk_op.is_read ? IDE_CMD_READ_SECTOR : IDE_CMD_WRITE_SECTOR);

    if(!devide_wait(IDE_STATUS_DATAREQUEST))
        goto fail;

    if (blk_op.is_read)
	devide_read_data();
    else {
	devide_write_data();
	if(!devide_wait(IDE_STATUS_READY))
	    goto fail;
	blk_op.blkdev->driver_data |= FLAG_CACHE_DIRTY;
    }

    ide_deselect();

    return 1;
fail:
    ide_deselect();
    return 0;
}

int devide_flush_cache(void)
{
    uint8_t drive;

    drive = blk_op.blkdev->driver_data & DRIVE_NR_MASK;

    ide_select(drive);

    /* check drive has a cache and was written to since the last flush */
    if((blk_op.blkdev->driver_data & (FLAG_WRITE_CACHE | FLAG_CACHE_DIRTY))
		                 == (FLAG_WRITE_CACHE | FLAG_CACHE_DIRTY)){
	devide_writeb(ide_reg_lba_3, (((drive & 1) == 0) ? 0xE0 : 0xF0)); // select drive

	if (!devide_wait(IDE_STATUS_READY))
	    goto fail;

	devide_writeb(ide_reg_command, IDE_CMD_FLUSH_CACHE);

	if (!devide_wait(IDE_STATUS_READY))
	    goto fail;

        /* drive cache is now clean */
	blk_op.blkdev->driver_data &= ~FLAG_CACHE_DIRTY;
    }
    ide_deselect();
    return 0;

fail:
    udata.u_error = EIO;
    ide_deselect();
    return -1;
}

/****************************************************************************/
/* The innermost part of the transfer routines has to live in common memory */
/* since it must be able to bank switch to the user memory bank.            */
/****************************************************************************/
#ifndef IDE_REG_INDIRECT

#ifndef IDE_IS_MMIO

/* Port I/O: Currently Z80 only */

COMMON_MEMORY

void devide_read_data(void) __naked
{
    __asm
            ld a, (_blk_op+BLKPARAM_IS_USER_OFFSET) ; blkparam.is_user
            ld hl, (_blk_op+BLKPARAM_ADDR_OFFSET)   ; blkparam.addr
            ld b, #0                                ; setup count
            ld c, #IDE_REG_DATA                     ; setup port number
#ifdef SWAPDEV
	    cp #2
            jr nz, not_swapin
            ld a, (_blk_op+BLKPARAM_SWAP_PAGE)	    ; blkparam.swap_page
            call map_for_swap
            jr swapin
not_swapin:
#endif
            or a                                    ; test is_user
            call nz, map_process_always             ; map user memory first if required
swapin:
            inir                                    ; transfer first 256 bytes
            inir                                    ; transfer second 256 bytes
            or a                                    ; test is_user
            ret z                                   ; done if kernel memory transfer
            jp map_kernel                           ; else map kernel then return
    __endasm;
}

void devide_write_data(void) __naked
{
    __asm
            ld a, (_blk_op+BLKPARAM_IS_USER_OFFSET) ; blkparam.is_user
            ld hl, (_blk_op+BLKPARAM_ADDR_OFFSET)   ; blkparam.addr
            ld b, #0                                ; setup count
            ld c, #IDE_REG_DATA                     ; setup port number
#ifdef SWAPDEV
	    cp #2
            jr nz, not_swapout
            ld a, (_blk_op+BLKPARAM_SWAP_PAGE)	    ; blkparam.swap_page
            call map_for_swap
            jr swapout
not_swapout:
#endif
            or a                                    ; test is_user
            call nz, map_process_always             ; else map user memory first if required
swapout:
            otir                                    ; transfer first 256 bytes
            otir                                    ; transfer second 256 bytes
            or a                                    ; test is_user
            ret z                                   ; done if kernel memory transfer
            jp map_kernel                           ; else map kernel then return
    __endasm;
}
#endif
#endif
