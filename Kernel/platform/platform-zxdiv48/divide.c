#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <tinyide.h>
/*
 *	Might be worth unifying with the other versions but this one
 *	differs a little
 */
COMMON_MEMORY

void divide_read_data(uint8_t *p) __naked
{
    __asm
            pop bc
            pop de
            pop hl
            push hl
            push de
            push bc
            ld a, (_td_raw)			    ; user or not (no swap yet)
            ld bc, #0x00A3                    	    ; setup port number
                                                    ; and count
	    or a
            push af
            call nz, map_buffers
            ld a,#0x05
            out (0xfe),a
            inir                                    ; transfer first 256 bytes
            ld a,#0x02
            out (0xfe),a
            inir                                    ; transfer second 256 bytes
            ld a,(_vtborder)
            out (0xfe),a
            pop af
            ret z
            jp map_kernel_restore               ; else map kernel then return
    __endasm;
}

void divide_write_data(uint8_t *p) __naked
{
    __asm
            pop bc
            pop de
            pop hl
            push hl
            push de
            push bc
            ld a, (_td_raw)
            ld bc, #0x00A3	                    ; setup port number
                                                    ; and count
	    or a
            push af
            call nz, map_buffers
            ld a,#0x05
            out (0xfe),a
            otir                                    ; transfer first 256 bytes
            ld a,#0x02
            out (0xfe),a
            otir                                    ; transfer second 256 bytes
            ld a,(_vtborder)
            out (0xfe),a
            pop af
            ret z
            jp map_kernel_restore               ; else map kernel then return
    __endasm;
}
