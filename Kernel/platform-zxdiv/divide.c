#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devide.h>
#include <blkdev.h>
#include <platform_ide.h>
#include <devtty.h>

/* We have to provide slightly custom methods here because of the banked
   kernel */
COMMON_MEMORY

void devide_read_data(void) __naked
{
    __asm
            ld a, (_blk_op+BLKPARAM_IS_USER_OFFSET) ; blkparam.is_user
            ld hl, (_blk_op+BLKPARAM_ADDR_OFFSET)   ; blkparam.addr
            ld bc, #IDE_REG_DATA                    ; setup port number
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
            call nz, map_process_always  	    ; map user memory first if required
doread:
            ld a,#0x05
            out (0xfe),a
            inir                                    ; transfer first 256 bytes
            ld a,#0x02
            out (0xfe),a
            inir                                    ; transfer second 256 bytes
            ld a,(_vtborder)
            out (0xfe),a
            pop af
            or a
            jp nz, map_kernel_restore               ; else map kernel then return
            ret
    __endasm;
}

void devide_write_data(void) __naked
{
    __asm
            ld a, (_blk_op+BLKPARAM_IS_USER_OFFSET) ; blkparam.is_user
            ld hl, (_blk_op+BLKPARAM_ADDR_OFFSET)   ; blkparam.addr
            ld bc, #IDE_REG_DATA                    ; setup port number
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
            call nz, map_process_always             ; else map user memory first if required
dowrite:
            ld a,#0x05
            out (0xfe),a
            otir                                    ; transfer first 256 bytes
            ld a,#0x02
            out (0xfe),a
            otir                                    ; transfer second 256 bytes
            ld a,(_vtborder)
            out (0xfe),a
            pop af
            or a
            jp nz, map_kernel_restore               ; else map kernel then return
            ret
    __endasm;
}
