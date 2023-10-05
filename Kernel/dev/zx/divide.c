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
            ld bc, #IDE_REG_DATA                    ; setup port number
                                                    ; and count
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
