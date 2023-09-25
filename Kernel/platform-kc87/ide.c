#include <kernel.h>
#include <tinyide.h>
#include <plt_ide.h>

/* Port I/O: Currently Z80 only */

COMMON_MEMORY

void devide_read_data(uint8_t *ptr) __naked
{
    __asm
            pop bc
            pop de
            pop hl
            push hl
            push de
            push bc
            ld a, (_td_raw) ; blkparam.is_user
            ld bc, #IDE_REG_DATA                    ; setup port number
                                                    ; and count
#ifdef SWAPDEV
	    cp #2
            jr nz, not_swapin
            ld a, (_td_page)	    ; blkparam.swap_page
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
            inir
            inir
            jp map_kernel_restore                   ; else map kernel then return
    __endasm;
}

void devide_write_data(uint8_t *ptr) __naked
{
    __asm
            pop bc
            pop de
            pop hl
            push hl
            push de
            push bc
            ld a, (_td_raw) 			    ; blkparam.is_user
            ld bc, #IDE_REG_DATA                    ; setup port number
                                                    ; and count
#ifdef SWAPDEV
	    cp #2
            jr nz, not_swapout
            ld a, (_td_page)			    ; blkparam.swap_page
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
            otir
            otir
            jp map_kernel_restore                   ; else map kernel then return
    __endasm;
}
