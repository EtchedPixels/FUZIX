#define _IDE_PRIVATE

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <timer.h>
#include <devide.h>
#include <blkdev.h>

/*
 *	We need slightly custom transfer routines for the IDE controller
 *	as we have no common stack to use.
 */

COMMON_MEMORY

void devide_read_data(void) __naked
{
    __asm
            ld a, (_blk_op+BLKPARAM_IS_USER_OFFSET) ; blkparam.is_user
            ld hl, (_blk_op+BLKPARAM_ADDR_OFFSET)   ; blkparam.addr
            ld bc, #IDE_REG_DATA                    ; setup port number
                                                    ; and count
	    cp #2
            jr nz, not_swapin
            ld a, (_blk_op+BLKPARAM_SWAP_PAGE)	    ; blkparam.swap_page
not_swapin:
            ; At this point A = 0 kernel, A = 1, user
            out (0x3E),a
            inir                                    ; transfer first 256 bytes
            inir                                    ; transfer second 256 bytes
            xor a
            out (0x3E),a
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
	    cp #2
            jr nz, not_swapout
            ld a, (_blk_op+BLKPARAM_SWAP_PAGE)	    ; blkparam.swap_page
not_swapout:
            ; At this point A = 0 kernel, A = 1, kernel
            out (0x3E),a
            otir                                    ; transfer first 256 bytes
            otir                                    ; transfer second 256 bytes
            xor a
            out (0x3E),a
            ret
    __endasm;
}
