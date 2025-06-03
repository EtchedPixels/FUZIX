#include <kernel.h>
#include <kdata.h>
#include <plt_ide.h>

/* We have to provide slightly custom methods here because of the banked
   kernel */

#ifdef CONFIG_TD_IDE
COMMON_MEMORY

void devide_read_data(uint8_t *p) __naked
{
    __asm
    	.globl a_map_to_bc
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
#ifdef SWAPDEV
	        cp #2
            jr nz, not_swapin
            ld a, (_td_page)	 		   ; swap page
            call map_for_swap
            jr doread
not_swapin:
#endif
            or a                                    ; test is_user
            jr z, doread		    ; map user memory first if required
            ld a, (_td_page)
            call a_map_to_bc
            out (c),c
doread:
            ld bc,#0x7f10
            out (c),c
            ld c,#0x51 ; Sea Green
            out (c),c
            ld bc, #IDE_REG_DATA                    ; setup port number
                                                    ; and count
            ld a,#64
doread1:                    ; transfer 512 bytes
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
            ld bc,#0x7fc2
            out (c),c
            ld bc,#0x7f10
            out (c),c                                    
            ld a,(_vtborder)
            out (c),a
            ret
    __endasm;
}

void devide_write_data(uint8_t *p) __naked
{
    __asm
	.globl a_map_to_bc
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
                                                    ; and count
#ifdef SWAPDEV
	    cp #2
            jr nz, not_swapout
            ld a, (_td_page)			    ; page to swap
            call map_for_swap
            jr dowrite
not_swapout:
#endif
            or a                                    ; test is_user
            jr z, dowrite		    ; map user memory first if required
            ld a, (_td_page)
            call a_map_to_bc
            out (c),c
dowrite:

            ld bc,#0x7f10
            out (c),c
            ld c,#0x45  ;Purple
            out (c),c
            ld bc, #IDE_REG_DATA                    ; setup port number
                                                    ; and count
            ld a,#64
dowrite1:                    ; transfer 512 bytes
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
            ld bc,#0x7fc2
            out (c),c
            ld bc,#0x7f10
            out (c),c                                    
            ld a,(_vtborder)
            out (c),a
            ret
    __endasm;
}
#endif
