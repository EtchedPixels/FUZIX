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
	.area _SERIALDATA
        .area _DATA
        .area _BSEG
        .area _BSS
        .area _HEAP
        ; note that areas below here may be overwritten by the heap at runtime, so
        ; put initialisation stuff in here
        .area _GSINIT      ; unused
        .area _GSFINAL     ; unused
        .area _DISCARD
        .area _INITIALIZER ; binman copies this to the right place for us
        .area _COMMONMEM
	.area _SERIAL

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
        .globl kstack_top

	.include "kernel.def"

	; Dummy page0 area so binman doesn't pack us

	.area _PAGE0

        ; startup code
        .area _CODE
init:                       ; must be at 0x0100 as we are loaded at that
	di

	; setup the memory paging for kernel
        ld a, #33
        out (MPGSEL_1), a       ; map page 33 at 0x4000
        inc a
        out (MPGSEL_2), a       ; map page 34 at 0x8000
	inc a
        out (MPGSEL_3), a       ; map page 35 at 0xC000

mappedok:
        ; switch to stack in high memory
        ld sp, #kstack_top

        ; Zero the data area
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

	.area _BOOT
	jp init
