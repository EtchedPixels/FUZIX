; 2013-12-18 William R Sowerbutts

        .module crt0

        ; Ordering of segments for the linker.
        ; WRS: Note we list all our segments here, even though
        ; we don't use them all, because their ordering is set
        ; when they are first seen.

	; Stuff that ends up in RAM, initialized bits first then data
	; We will need to pull the initialized bits into ROM for crt0.s
	; to ldir down
        .area _COMMONMEM
        .area _CONST
        .area _INITIALIZED
	.area _STUBS
        .area _DATA
        .area _BSEG
        .area _BSS
        .area _HEAP
        .area _GSINIT
        .area _GSFINAL
	; Udata ends up just below program code (so we can swap it easily)
	.area _UDATA
	; First ROM is CODE (CODE2 folded in)
        .area _CODE
	; Second ROM is CODE3 VIDEO FONT and initialized data to copy down
	.area _CODE3
	.area _FONT
	.area _VIDEO
        .area _INITIALIZER
	; Discard needs splitting code/data!
	.area _DISCARD
	; and the bitmap display lives at E000-FFFF


        ; imported symbols
        .globl _fuzix_main
        .globl init_early
        .globl init_hardware
        .globl s__DATA
        .globl l__DATA
        .globl kstack_top

        ; startup code
        .area _CODE
init:
        di
        ld sp, #kstack_top

        ; Configure memory map
        call init_early

	; zero the data area
	ld hl, #s__DATA
	ld de, #s__DATA + 1
	ld bc, #l__DATA - 1
	ld (hl), #0
	ldir

        ; Hardware setup
        call init_hardware

        ; Call the C main routine
        call _fuzix_main
    
        ; main shouldn't return, but if it does...
        di
stop:   halt
        jr stop

	.area _STUBS
stubs:
	.ds 768
