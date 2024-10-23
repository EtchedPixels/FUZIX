#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <tinydisk.h>
#include <plt_ch375.h>
#include <devtty.h>

/* We have to provide slightly custom methods here because of the banked
   kernel */
COMMON_MEMORY

void ch375_rblock(uint8_t *p) __naked
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
            ld c,#0x4a  ;yellow
            out (c),c
            ld bc, #CH376_REG_DATA                    ; setup port number
                                                    ; and count
                    ; transfer first 32 bytes
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
            ;push bc
            ;ld bc,#0x7f10
            ;out (c),c
            ;ld c,#0x4c
            ;out (c),c
                      ; transfer second 32 bytes
            ;pop bc
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

void ch375_wblock(uint8_t *p) __naked
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
            ld bc, #CH376_REG_DATA                    ; setup port number
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
            ld c,#0x4e  ;orange
            out (c),c
            ld bc, #CH376_REG_DATA                    ; setup port number
                                                    ; and count
                        ; transfer first 256 bytes
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
            ;push bc
            ;ld bc,#0x7f10
            ;out (c),c
            ;ld c,#0x55
            ;out (c),c                                   
            ;pop bc
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
