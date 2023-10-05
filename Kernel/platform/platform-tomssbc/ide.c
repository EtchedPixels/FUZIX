#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <timer.h>
#include <tinyide.h>
#include <plt_ide.h>

/*
 *	We need slightly custom transfer routines for the IDE controller
 *	as we have no common stack to use.
 */

COMMON_MEMORY

void devide_read_data(uint8_t *p) __naked
{
    __asm
            pop de
            pop hl
            push hl
            push de
            ld bc, #IDE_REG_DATA	; port and count
            ld a, (_td_raw)
	    cp #2
            jr nz, not_swapin
            ld a, (_td_page)			    ; swap page
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

void devide_write_data(uint8_t *p) __naked
{
    __asm
            pop de
            pop hl
            push hl
            push de
            ld bc, #IDE_REG_DATA	; port and count
            ld a, (_td_raw)
	    cp #2
            jr nz, not_swapout
            ld a, (_td_page)			    ; swap page
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
