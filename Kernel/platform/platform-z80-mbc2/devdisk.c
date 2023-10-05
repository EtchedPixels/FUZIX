#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <blkdev.h>
#include <mbc2.h>

static uint8_t last_drive = 0xFF;

/*
 *	Simple virtual drives (no raw SD access alas)
 */

uint_fast8_t vd_transfer_sector(void)
{
    irqflags_t irq;
    uint_fast8_t drive = blk_op.blkdev->driver_data & VD_DRIVE_NR_MASK;
    uint16_t track;
    uint8_t sector;
    uint8_t c;

    irq = di();

    if (drive != last_drive) {
        opcode = OP_SET_DISK;
        opwrite = drive;
        last_drive = drive;
        opcode = OP_GET_ERROR;
        c = opread;
        if (c) {
            kprintf("hd: drive %d select failed %d.\n", drive, c);
            last_drive = 255;
            irqrestore(irq);
            return 0;
        }
    }

    track = blk_op.lba >> 5;
    sector = blk_op.lba & 0x1F;

    opcode = OP_SET_TRACK;
    opwrite = track;
    opwrite = track >> 8;
    opcode = OP_SET_SECTOR;
    opwrite = sector;

    if (blk_op.is_read)
        vd_read();
    else
        vd_write();

    opcode = OP_GET_ERROR;
    c = opread;
    irqrestore(irq);

    if (c) {
        kprintf("hd: drive %d error %d.\n", drive, c);
        return 0;
    }
    return 1;
}

int vd_flush(void)
{
    return 0;
}

void vd_init_drive(uint_fast8_t drive)
{
    blkdev_t *blk;
    irqflags_t irq;

    irq = di();
    opcode = OP_SET_DISK;
    opwrite = drive;
    opcode = OP_GET_ERROR;
    if (opread == 0) {
        blk = blkdev_alloc();
        if (blk) {
            blk->transfer = vd_transfer_sector;
            blk->flush = vd_flush;
            blk->driver_data = drive & VD_DRIVE_NR_MASK;
            /* Fixed size */
            blk->drive_lba_count = 8UL << 20;
            blkdev_scan(blk, SWAPSCAN);
        }
    }
    last_drive = 255;
    irqrestore(irq);
}

void vd_init(void)
{
    uint_fast8_t d;
    for (d = 0; d < VD_DRIVE_COUNT; d++)
        vd_init_drive(d);
}        

COMMON_MEMORY

void vd_read(void) __naked
{
    __asm
            ld a, (_blk_op+BLKPARAM_IS_USER_OFFSET) ; blkparam.is_user
            ld hl, (_blk_op+BLKPARAM_ADDR_OFFSET)   ; blkparam.addr
            ld bc, #OP_RD_PORT                      ; setup port number
                                                    ; and count
            push af
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
            ld a,#OP_READ_SECTOR
            out (OP_PORT),a
            inir                                    ; transfer first 256 bytes
            inir                                    ; transfer second 256 bytes
            pop af
            or a
            ret z
            jp map_kernel                           ; else map kernel then return
    __endasm;
}

void vd_write(void) __naked
{
    __asm
            ld a, (_blk_op+BLKPARAM_IS_USER_OFFSET) ; blkparam.is_user
            ld hl, (_blk_op+BLKPARAM_ADDR_OFFSET)   ; blkparam.addr
            ld bc, #OP_WR_PORT                      ; setup port number
                                                    ; and count
            push af
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
            call map_proc_always                    ; else map user memory first if required
            jr dowrite
wr_kernel:
            call map_buffers
dowrite:
            ld a,#OP_WRITE_SECTOR
            out (OP_PORT),a
            otir                                    ; transfer first 256 bytes
            otir                                    ; transfer second 256 bytes
            pop af
            or a
            ret z
            jp map_kernel                           ; else map kernel then return
    __endasm;
}
