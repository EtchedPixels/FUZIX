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
 *
 *	As we only have one bank of user it's a bit simpler
 */

COMMON_MEMORY

#undef ei
#undef di

void devide_read_data(void) __naked
{
    __asm
            ld a, (_blk_op+BLKPARAM_IS_USER_OFFSET) ; blkparam.is_user
            ld hl, (_blk_op+BLKPARAM_ADDR_OFFSET)   ; blkparam.addr
                                                    ; and count
	    or a
            jr z, via_kernel
            di
            ld bc,(sio_reg)
            out (c),b
            ld a,(bits_to_user + 1)
            out (c),a
via_kernel:
            ld bc, #IDE_REG_DATA                    ; setup port number
            inir                                    ; transfer first 256 bytes
            inir                                    ; transfer second 256 bytes
            ld bc,(sio_reg)
            out (c),b
            ld a,(bits_to_user)
            out (c),a
            ld a,(_int_disabled)
            or a
            ret z
            ei
            ret
    __endasm;
}

void devide_write_data(void) __naked
{
    __asm
            ld a, (_blk_op+BLKPARAM_IS_USER_OFFSET) ; blkparam.is_user
            ld hl, (_blk_op+BLKPARAM_ADDR_OFFSET)   ; blkparam.addr
                                                    ; and count
	    or a
            jr z, wvia_kernel
            di
            ld bc,(sio_reg)
            out (c),b
            ld a,(bits_to_user + 1)
            out (c),a
wvia_kernel:            
            ld bc, #IDE_REG_DATA                    ; setup port number
            otir                                    ; transfer first 256 bytes
            otir                                    ; transfer second 256 bytes
            ld bc,(sio_reg)
            out (c),b
            ld a,(bits_to_user )
            out (c),a
            ld a,(_int_disabled)
            or a
            ret z
            ei
            ret
    __endasm;
}
