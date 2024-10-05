/*
 *	The Memotech MTX systems have two totally incompatible
 *	IDE interface designs. As a result we need to wrap the IDE driver
 */

#include <kernel.h>
#include <printf.h>
#include <blkdev.h>
#include <devide.h>
#include <ppide.h>

#define CFX2_REG_DATA	0xB0

uint8_t has_cfx2;
uint8_t has_cfx1;

uint8_t probe_cfx2(void)
{
    out(0xB2, 0xAA);
    if (in(0xB2) != 0xAA)
        return 0;
    out(0xB2, 0x55);
    if (in(0xB2) != 0x55)
        return 0;
    has_cfx2 = 1;
    kputs("CFX-II detected.\n");
    return 1;
}

/* This works because our ide_reg identifiers for PPIDE have the A2-A0 bits
   in the low 3 bits of reg as expected */

uint8_t devide_readb(uint8_t reg)
{
    if (has_cfx2 == 0)
        return ppide_readb(reg);
    else
        return in(0xB0 + (reg & 7));
}

void devide_writeb(uint8_t reg, uint8_t val)
{
    if (has_cfx2 == 0)
        return ppide_writeb(reg, val);
    else
        out(0xB0 + (reg & 7), val);
}

extern void cfx2_read_data(void);
extern void cfx2_write_data(void);

void devide_read_data(void)
{
    if (has_cfx2 == 0) {
        ppide_read_data();
        return;
    }
    cfx2_read_data();
}

void devide_write_data(void)
{
    if (has_cfx2 == 0) {
        ppide_write_data();
        return;
    }
    cfx2_write_data();
}

COMMON_MEMORY

void cfx2_read_data(void) __naked
{
    __asm
            ld a, (_blk_op+BLKPARAM_IS_USER_OFFSET) ; blkparam.is_user
            ld hl, (_blk_op+BLKPARAM_ADDR_OFFSET)   ; blkparam.addr
            ld bc, #CFX2_REG_DATA                    ; setup port number
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
            jp map_kernel                           ; else map kernel then return
    __endasm;
}

void cfx2_write_data(void) __naked
{
    __asm
            ld a, (_blk_op+BLKPARAM_IS_USER_OFFSET) ; blkparam.is_user
            ld hl, (_blk_op+BLKPARAM_ADDR_OFFSET)   ; blkparam.addr
            ld bc, #CFX2_REG_DATA                    ; setup port number
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
            call map_proc_always                    ; else map user memory first if required
            jr dowrite
wr_kernel:
            call map_buffers
dowrite:
            otir                                    ; transfer first 256 bytes
            otir                                    ; transfer second 256 bytes
            jp map_kernel                           ; else map kernel then return
    __endasm;
}
