#include "kernelu.def"

	.code

init:
        di
        ; switch to stack in high memory
        ld sp, kstack_top

        ; move the common memory where it belongs    
        ld hl, __bss
        ld de, __common
        ld bc, __common_size
        ldir
	; font
	ld de, __FONT
	ld bc, l__FONT
	ldir
        ; and the discard
        ld de, __discard
        ld bc, __discard_size
        ldir

        ld hl, __bss
        ld de, __bss + 1
        ld bc, __bss_size - 1
        ld (hl), 0
        ldir

        ; Hardware setup
        call init_hardware

        ; Call the C main routine
        call _fuzix_main
    
        ; fuzix_main() shouldn't return, but if it does...
        di
stop:   halt
        jr stop
