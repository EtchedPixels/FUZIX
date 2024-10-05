#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <tinyide.h>
#include <plt_ide.h>

/* We have to provide slightly custom methods here because of the banked
   kernel */
COMMON_MEMORY

void devide_read_data(uint8_t *data) __naked
{
    __asm
            pop bc
            pop de
            pop hl
            push hl
            push de
            push bc
            ld a, (_td_raw)
            ld bc, #IDE_REG_DATA_LATCH              ; setup port number
                                                    ; and count
            push af
#ifdef SWAPDEV
	    cp #2
            jr nz, not_swapin
            ld a, (_td_page)
            call map_for_swap
            jr doread
not_swapin:
#endif
            or a                                    ; test is_user
            call nz, map_proc_always  	    ; map user memory first if required
doread:
readword:
            in a,(0x10)				    ; Read a word
            ld (hl),a				    ; Store the byte we got
            inc hl
            ini					    ; Now store the latched byte
            jr nz, readword
            pop af
            or a
            ret z
            jp map_kernel_restore               ; else map kernel then return
    __endasm;
}

void devide_write_data(uint8_t *data) __naked
{
    /* This assumes we are banked */
    __asm
            pop bc
            pop de
            pop hl
            push hl
            push de
            push bc
            ld a, (_td_raw)
            ld bc, #IDE_REG_DATA_LATCH              ; setup port number
                                                    ; and count
            push af
#ifdef SWAPDEV
	    cp #2
            jr nz, not_swapout
            ld a, (_td_page)
            call map_for_swap
            jr dowrite
not_swapout:
#endif
            or a                                    ; test is_user
            call nz, map_proc_always             ; else map user memory first if required
dowrite:
            ; Write is somewhat ugly
writeword:
            ld a,(hl)
            inc hl				    ; data byte
            outi				    ; latch a byte
            out (0x10),a			    ; write the data
                                                    ; and thus the latch
            jr nz, writeword
            pop af
            or a
            ret z
            jp  map_kernel_restore               ; else map kernel then return
    __endasm;
}
