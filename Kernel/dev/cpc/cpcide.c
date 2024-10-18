#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <tinyide.h>
#include <plt_ide.h>
#include <devtty.h>

/* We have to provide slightly custom methods here because of the banked
   kernel */
COMMON_MEMORY

void devide_read_data(uint8_t *p) __naked
{
    __asm
#ifdef CONFIG_BANKED
            pop bc
            pop de
            pop hl
            push hl
            push de
            push bc
#else
            pop de
            pop hl
            push hl
            push de
#endif
            ld a, (_td_raw) 			    ; I/O type ?
            
            push af
#ifdef SWAPDEV
	    cp #2
            jr nz, not_swapin
            ld a, (_td_page)	 		   ; swap page
            call map_for_swap
            jr doread
not_swapin:
#endif
            or a                                    ; test is_user
            call nz, map_proc_always		    ; map user memory first if required
doread:
            ld bc,#0x7f10
            out (c),c
            ld c,#0x53
            out (c),c
            ld bc, #IDE_REG_DATA                    ; setup port number
                                                    ; and count
            ld a,#8
doread1:                    ; transfer first 256 bytes
            ini
            inc b
            ini
            inc b
            ini
            inc b
            ini
            inc b
            ini
            inc b
            ini
            inc b
            ini
            inc b
            ini
            inc b
            ini
            inc b
            ini
            inc b
            ini
            inc b
            ini
            inc b
            ini
            inc b
            ini
            inc b
            ini
            inc b
            ini
            inc b
            ini
            inc b
            ini
            inc b
            ini
            inc b
            ini
            inc b
            ini
            inc b
            ini
            inc b
            ini
            inc b
            ini
            inc b
            ini
            inc b
            ini
            inc b
            ini
            inc b
            ini
            inc b
            ini
            inc b
            ini
            inc b
            ini
            inc b
            ini
            inc b
            dec a
            jr nz,doread1
            ld bc,#0x7f10
            out (c),c
            ld c,#0x4c
            out (c),c
            ld a,#8
doread2:                    ; transfer second 256 bytes
            ini
            inc b
            ini
            inc b
            ini
            inc b
            ini
            inc b
            ini
            inc b
            ini
            inc b
            ini
            inc b
            ini
            inc b
            ini
            inc b
            ini
            inc b
            ini
            inc b
            ini
            inc b
            ini
            inc b
            ini
            inc b
            ini
            inc b
            ini
            inc b
            ini
            inc b
            ini
            inc b
            ini
            inc b
            ini
            inc b
            ini
            inc b
            ini
            inc b
            ini
            inc b
            ini
            inc b
            ini
            inc b
            ini
            inc b
            ini
            inc b
            ini
            inc b
            ini
            inc b
            ini
            inc b
            ini
            inc b
            ini
            inc b
            dec a
            jr nz,doread2
            ld bc,#0x7f10
            out (c),c                                    
            ld a,(_vtborder)
            out (c),a
            pop af
            or a
            jp nz, map_kernel_restore               ; else map kernel then return
            ret
    __endasm;
}

void devide_write_data(uint8_t *p) __naked
{
    __asm
#ifdef CONFIG_BANKED
            pop bc
            pop de
            pop hl
            push hl
            push de
            push bc
#else
            pop de
            pop hl
            push hl
            push de
#endif
            ld a, (_td_raw) ;			    ; I/O type
            ld bc, #IDE_REG_DATA                    ; setup port number
                                                    ; and count
            push af
#ifdef SWAPDEV
	    cp #2
            jr nz, not_swapout
            ld a, (_td_page)			    ; page to swap
            call map_for_swap
            jr dowrite
not_swapout:
#endif
            or a                                    ; test is_user
            call nz, map_proc_always                ; else map user memory first if required
dowrite:

            ld bc,#0x7f10
            out (c),c
            ld c,#0x55
            out (c),c
            ld bc, #IDE_REG_DATA                    ; setup port number
                                                    ; and count
            ld a,#8
dowrite1:                    ; transfer first 256 bytes
            inc b
            outi
            inc b
            outi
            inc b
            outi
            inc b
            outi
            inc b
            outi
            inc b
            outi
            inc b
            outi
            inc b
            outi
            inc b
            outi
            inc b
            outi
            inc b
            outi
            inc b
            outi
            inc b
            outi
            inc b
            outi
            inc b
            outi
            inc b
            outi
            inc b
            outi
            inc b
            outi
            inc b
            outi
            inc b
            outi
            inc b
            outi
            inc b
            outi
            inc b
            outi
            inc b
            outi
            inc b
            outi
            inc b
            outi
            inc b
            outi
            inc b
            outi
            inc b
            outi
            inc b
            outi
            inc b
            outi
            inc b
            outi
            dec a
            jr nz,dowrite1
            ld bc,#0x7f10
            out (c),c
            ld c,#0x4e
            out (c),c                                   
            ld a,#8
dowrite2:
            inc b
            outi
            inc b
            outi
            inc b
            outi
            inc b
            outi
            inc b
            outi
            inc b
            outi
            inc b
            outi
            inc b
            outi
            inc b
            outi
            inc b
            outi
            inc b
            outi
            inc b
            outi
            inc b
            outi
            inc b
            outi
            inc b
            outi
            inc b
            outi
            inc b
            outi
            inc b
            outi
            inc b
            outi
            inc b
            outi
            inc b
            outi
            inc b
            outi
            inc b
            outi
            inc b
            outi
            inc b
            outi
            inc b
            outi
            inc b
            outi
            inc b
            outi
            inc b
            outi
            inc b
            outi
            inc b
            outi
            inc b
            outi
            dec a
            jr nz,dowrite2
            ld bc,#0x7f10
            out (c),c                                    
            ld a,(_vtborder)
            out (c),a
            pop af
            or a
            jp nz, map_kernel_restore               ; else map kernel then return
            ret
    __endasm;
}
