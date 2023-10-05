#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <blkdev.h>
#include <bios.h>

static uint8_t last_drive = 0xFF;

/*
 *	Simple disk interface for FuzixBIOS.
 *
 *	Would be good to allow multi-sector I/O here.
 *
 *	FIXME: need to be able to hook open for rescan on media change
 *	and also to error if no media and not O_NDELAY- needs blkdev
 *	tweaks ?
 */

uint_fast8_t vd_transfer_sector(void)
{
    uint_fast8_t drive = blk_op.blkdev->driver_data & VD_DRIVE_NR_MASK;
    uint16_t c;

    if (drive != last_drive) {
        c = fuzixbios_disk_select(drive);
        if (c) {
            kprintf("hd: drive %d select failed %d.\n", drive, c);
            last_drive = 255;
            return 0;
        }
    }

    c = fuzixbios_disk_set_lba(&blk_op.lba);
    if (c == 0) {
        if (blk_op.is_read)
            c = vd_read();
        else
            c = vd_write();
    }

    /* Low 8 bits are the Fuzix error code, high are the device specific info */
    if (c) {
        udata.u_error = c & 0xFF;
        /* EROFS is not a kernel loggable event */
        if ((c & 0xFF) != EROFS) {
            kprintf("hd: drive %d error %d (%d).\n", drive, c & 0xFF, c >> 8);
        }
        return 0;
    }
    return 1;
}

int vd_flush(void)
{
    if (fuzixbios_disk_flush(&blk_op)) {
        udata.u_error = EIO;
        return -1;
    }
    return 0;
}

uint16_t callback_disk(uint16_t event) __z88dk_fastcall
{
    uint8_t unit = event >> 8;
    uint8_t op = event;
    
    /* Not yet supported: but we need to handle media change notifications */
    /* We can use bdrop for some of it but not here - this is a callback
       so we need to set flags only, then allow blkdev open methods so that
       a new open() can call a new method to ack the media change */
    return 0;
}

void vd_init_drive(uint_fast8_t drive)
{
    blkdev_t *blk;
    struct fuzixbios_diskparam *dp;

    /* FIXME: add floppies to /dev/fd */
    if (fuzixbios_disk_select(drive) == 0) {
        dp = fuzixbios_disk_param();
        blk = blkdev_alloc();
        if (blk) {
            blk->transfer = vd_transfer_sector;
            blk->flush = vd_flush;
            blk->driver_data = drive & VD_DRIVE_NR_MASK;
            blk->drive_lba_count = dp->blocks;
            if (dp->flags & DP_PARTITION)
                blkdev_scan(blk, (dp->flags & DP_SWAPPABLE) ? SWAPSCAN : 0);
        }
    }
    last_drive = 255;
}

void vd_init(void)
{
    uint_fast8_t d;
    for (d = 0; d < biosinfo->num_disk; d++)
        vd_init_drive(d);
}        

COMMON_MEMORY

uint8_t vd_read(void) __naked
{
    __asm
            ld a, (_blk_op+BLKPARAM_IS_USER_OFFSET) ; blkparam.is_user
            ld hl, (_blk_op+BLKPARAM_ADDR_OFFSET)   ; blkparam.addr
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
            call map_process_always  	            ; map user memory first if required
            jr doread
rd_kernel:
            call map_buffers
doread:
            call _fuzixbios_disk_read
            pop af
            or a
            ret z
            jp map_kernel                           ; else map kernel then return
    __endasm;
}

uint8_t vd_write(void) __naked
{
    __asm
            ld a, (_blk_op+BLKPARAM_IS_USER_OFFSET) ; blkparam.is_user
            ld hl, (_blk_op+BLKPARAM_ADDR_OFFSET)   ; blkparam.addr
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
            call map_process_always                 ; else map user memory first if required
            jr dowrite
wr_kernel:
            call map_buffers
dowrite:
            call _fuzixbios_disk_write
            pop af
            or a
            ret z
            jp map_kernel                           ; else map kernel then return
    __endasm;
}
