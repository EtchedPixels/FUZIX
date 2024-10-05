#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devide.h>
#include <blkdev.h>
#include <platform_ide.h>
#include <devtty.h>

/*
 *	Might be worth unifying with the other versions but this one
 *	differs a little
 */
COMMON_MEMORY

void divide_read_data(void) __naked
{
    __asm
            ld a, (_blk_op+BLKPARAM_IS_USER_OFFSET) ; blkparam.is_user
            ld hl, (_blk_op+BLKPARAM_ADDR_OFFSET)   ; blkparam.addr
            ld bc, #0x00A3                    	    ; setup port number
                                                    ; and count
	    cp #1
            push af
            call z, map_buffers
            ld a,#0x05
            out (0xfe),a
            inir                                    ; transfer first 256 bytes
            ld a,#0x02
            out (0xfe),a
            inir                                    ; transfer second 256 bytes
            ld a,(_vtborder)
            out (0xfe),a
            pop af
            ret nz
            jp map_kernel_restore               ; else map kernel then return
    __endasm;
}

void divide_write_data(void) __naked
{
    __asm
            ld a, (_blk_op+BLKPARAM_IS_USER_OFFSET) ; blkparam.is_user
            ld hl, (_blk_op+BLKPARAM_ADDR_OFFSET)   ; blkparam.addr
            ld bc, #0x00A3	                    ; setup port number
                                                    ; and count
	    cp #1
            push af
            call z, map_buffers
            ld a,#0x05
            out (0xfe),a
            otir                                    ; transfer first 256 bytes
            ld a,#0x02
            out (0xfe),a
            otir                                    ; transfer second 256 bytes
            ld a,(_vtborder)
            out (0xfe),a
            pop af
            ret nz
            jp map_kernel_restore               ; else map kernel then return
    __endasm;
}
