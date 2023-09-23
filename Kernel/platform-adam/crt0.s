; 2015-02-20 Sergey Kiselev
; 2013-12-18 William R Sowerbutts

        .module crt0

        ; Ordering of segments for the linker.
        ; WRS: Note we list all our segments here, even though
        ; we don't use them all, because their ordering is set
        ; when they are first seen.
        .area _CODE
        .area _HOME     ; compiler stores __mullong etc in here if you use them
        .area _CODE2
        .area _CONST
        .area _INITIALIZED
        .area _DATA
        .area _BSEG
        .area _BSS
        .area _HEAP
        ; note that areas below here may be overwritten by the heap at runtime, so
        ; put initialisation stuff in here
        .area _BUFFERS     ; _BUFFERS grows to consume all before it (up to KERNTOP)
        .area _GSINIT      ; unused
        .area _GSFINAL     ; unused
        .area _DISCARD
	.area _DISCARDL
	.area _DISCARDH
	.area _FONT
        .area _INITIALIZER ; binman copies this to the right place for us
        .area _COMMONMEM

        ; exported symbols
        .globl init

        ; imported symbols
        .globl _fuzix_main
        .globl init_hardware
        .globl s__INITIALIZER
        .globl s__COMMONMEM
        .globl l__COMMONMEM
        .globl s__DISCARD
        .globl l__DISCARD
        .globl s__DATA
        .globl l__DATA
        .globl s__FONT
        .globl l__FONT
        .globl kstack_top

	.include "kernel.def"

        ; startup code (0x100)
        .area _CODE

init:
        di
        ; switch to stack in high memory
        ld sp, #kstack_top

        ; move the common memory where it belongs    
        ld hl, #s__DATA
        ld de, #s__COMMONMEM
        ld bc, #l__COMMONMEM
        ldir
	; font
	ld de, #s__FONT
	ld bc, #l__FONT
	ldir
        ; and the discard
        ld de, #s__DISCARD
        ld bc, #l__DISCARD
        ldir

        ld hl, #s__DATA
        ld de, #s__DATA + 1
        ld bc, #l__DATA - 1
        ld (hl), #0
        ldir

        ; Hardware setup
        call init_hardware

        ; Call the C main routine
        call _fuzix_main
    
        ; fuzix_main() shouldn't return, but if it does...
        di
stop:   halt
        jr stop
