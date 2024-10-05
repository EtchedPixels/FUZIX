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

#ifdef CONFIG_IDE

bool devide_wait(uint_fast8_t bits)
{
    uint_fast8_t status;
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
            if (status & IDE_STATUS_ERROR)
                kprintf("ide error, code=%x\n", devide_readb(ide_reg_error));
            return false;
        }

        if(timer_expired(timeout)){
            kprintf("ide timeout, status=%x\n", status);
            return false;
        }
    }
}

#ifdef CONFIG_IDE_CHS

uint8_t ide_spt[IDE_DRIVE_COUNT];
uint8_t ide_heads[IDE_DRIVE_COUNT];
uint16_t ide_cyls[IDE_DRIVE_COUNT];

uint32_t last_lba_b;
uint8_t last_minor = 0xFF;
uint8_t last_head;
uint16_t last_cyl;

static void ide_setup_block(uint_fast8_t drive)
{
    uint32_t sector = blk_op.lba;
    uint32_t head;
    uint16_t cyl;

    /* Avoid the expensive 32bit maths (on 8bit anyway) by spotting
       further requests on the same head/cylinder as for those we just
       need to adjust the sector register */
    if (drive == last_minor && sector >= last_lba_b &&
        sector < last_lba_b + ide_spt[drive]) {
        head = last_head;
        cyl = last_cyl;
        sector = blk_op.lba - last_lba_b;
    } else {
        head = sector / ide_spt[drive];
        cyl = head / ide_heads[drive];

        sector %= ide_spt[drive];
        head %= ide_heads[drive];
        head |= 0xA0;

        if (drive & 1)
            head |= 0x10;

        last_minor = drive;
        last_lba_b = blk_op.lba - sector;	/* LBA of first block of this C/H */
        last_head = head;
        last_cyl = cyl;
    }

    /* Sector is 1 based */
    sector++;
    devide_writeb(ide_reg_lba_3, head);
    devide_writeb(ide_reg_lba_2, ((uint16_t)cyl) >> 8);
    devide_writeb(ide_reg_lba_1, cyl);
    devide_writeb(ide_reg_lba_0, sector);

/* kprintf("\nsb %u %u %u\n", (unsigned)cyl, (unsigned)head, (unsigned)sector); */
}

#else
static void ide_setup_block(uint_fast8_t drive)
{
#if defined(__SDCC_z80) || defined(__SDCC_z180) || defined(__SDCC_gbz80) || defined(__SDCC_r2k) || defined(__SDCC_r3k)
    /* sdcc sadly unable to figure this out for itself yet */
    uint8_t *p  = ((uint8_t *)&blk_op.lba)+3;
    devide_writeb(ide_reg_lba_3, (*(p--) & 0x0F) | ((drive & 1) ? 0xF0 : 0xE0)); // select drive, start loading LBA
    devide_writeb(ide_reg_lba_2, *(p--));
    devide_writeb(ide_reg_lba_1, *(p--));
    devide_writeb(ide_reg_lba_0, *p);
#else
    devide_writeb(ide_reg_lba_3, ((blk_op.lba >> 24) & 0xF) | ((drive & 1) ? 0xF0 : 0xE0)); // select drive, start loading LBA
    devide_writeb(ide_reg_lba_2, (blk_op.lba >> 16));
    devide_writeb(ide_reg_lba_1, (blk_op.lba >> 8));
    devide_writeb(ide_reg_lba_0, blk_op.lba);
#endif
}
#endif

uint_fast8_t devide_transfer_sector(void)
{
    uint_fast8_t drive;

    drive = blk_op.blkdev->driver_data & IDE_DRIVE_NR_MASK;

    ide_select(drive);
    ide_setup_block(drive);

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
    uint_fast8_t drive;

    drive = blk_op.blkdev->driver_data & IDE_DRIVE_NR_MASK;

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
#if !defined(IDE_REG_INDIRECT) && !defined(IDE_NONSTANDARD_XFER)

#ifndef IDE_IS_MMIO

/* Port I/O: Currently Z80 only */

COMMON_MEMORY

void devide_read_data(void) __naked
{
    __asm
            ld a, (_blk_op+BLKPARAM_IS_USER_OFFSET) ; blkparam.is_user
            ld hl, (_blk_op+BLKPARAM_ADDR_OFFSET)   ; blkparam.addr
            ld bc, #IDE_REG_DATA                    ; setup port number
                                                    ; and count
#ifdef SWAPDEV
	    cp #2
            jr nz, not_swapin
            ld a, (_blk_op+BLKPARAM_SWAP_PAGE)	    ; blkparam.swap_page
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

void devide_write_data(void) __naked
{
    __asm
            ld a, (_blk_op+BLKPARAM_IS_USER_OFFSET) ; blkparam.is_user
            ld hl, (_blk_op+BLKPARAM_ADDR_OFFSET)   ; blkparam.addr
            ld bc, #IDE_REG_DATA                    ; setup port number
                                                    ; and count
#ifdef SWAPDEV
	    cp #2
            jr nz, not_swapout
            ld a, (_blk_op+BLKPARAM_SWAP_PAGE)	    ; blkparam.swap_page
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

#endif /* CONFIG_IDE */
