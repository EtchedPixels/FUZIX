#include <kernel.h>
#include <blkdev.h>
#include <devide.h>
#include <ppide.h>
#include <printf.h>

uint16_t ide_port = 0x0010;		/* 256 count, port 0x10 */
static uint8_t drive;
/* Port I/O: Currently Z80 only */

static void cf_read_data(void);
static void cf_write_data(void);

void ide_select(uint_fast8_t d)
{
    drive = d >> 1;
}

uint8_t devide_readb(uint_fast8_t reg)
{
    if (drive == 1)	/* PPIDE */
        return ppide_readb(reg);
    else if (reg & 0x08)
        return in(ide_port + (reg & 7));
    return 0x00;
}

void devide_writeb(uint_fast8_t reg, uint_fast8_t val)
{
    if (drive == 1)	/* PPIDE */
        ppide_writeb(reg, val);
    else if (reg & 0x08)
        out(ide_port + (reg & 7), val);
}

void devide_read_data(void)
{
    if (drive == 1)	/* PPIDE */
        ppide_read_data();
    else
        cf_read_data();
}

void devide_write_data(void)
{
    if (drive == 1)	/* PPIDE */
        ppide_write_data();
    else
        cf_write_data();
}

COMMON_MEMORY

static void cf_read_data(void) __naked
{
    __asm
            ld a, (_blk_op+BLKPARAM_IS_USER_OFFSET) ; blkparam.is_user
            ld hl, (_blk_op+BLKPARAM_ADDR_OFFSET)   ; blkparam.addr
            ld bc, (_ide_port)                      ; setup port number
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
            call map_process_always  	            ; map user memory first if required
            jr doread
rd_kernel:
            call map_buffers
doread:
            call sector_dma_in
            jp map_kernel_restore                   ; else map kernel then return
    __endasm;
}

static void cf_write_data(void) __naked
{
    __asm
            ld a, (_blk_op+BLKPARAM_IS_USER_OFFSET) ; blkparam.is_user
            ld hl, (_blk_op+BLKPARAM_ADDR_OFFSET)   ; blkparam.addr
            ld bc, (_ide_port)                      ; setup port number
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
            call map_process_always                 ; else map user memory first if required
            jr dowrite
wr_kernel:
            call map_buffers
dowrite:
            call sector_dma_out
            jp map_kernel_restore                   ; else map kernel then return
    __endasm;
}
