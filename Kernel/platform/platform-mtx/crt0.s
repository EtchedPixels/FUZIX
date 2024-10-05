        .module crt0

        ; Ordering of segments for the linker.
        ; WRS: Note we list all our segments here, even though
        ; we don't use them all, because their ordering is set
        ; when they are first seen.
        .area _CODE
        .area _CODE2
	.area _HOME
        .area _CONST
	.area _VIDEO
        .area _INITIALIZED
        .area _DATA
        .area _BSEG
        .area _BSS
        .area _HEAP
	.area _BUFFERS
	; Buffers will overwrite everything up to commonmem
        .area _GSINIT
        .area _GSFINAL
	.area _DISCARD
        .area _INITIALIZER
	; We load the font into the VDP then it's discardable
	.area _FONT
	; and the common memory goes top
        .area _COMMONMEM
        .area _COMMONDATA

        ; imported symbols
        .globl _fuzix_main
        .globl init_early
        .globl init_hardware
        .globl s__DATA
        .globl l__DATA
        .globl s__FONT
        .globl l__FONT
        .globl s__BUFFERS
        .globl l__BUFFERS
        .globl s__DISCARD
        .globl l__DISCARD
        .globl s__INITIALIZER
        .globl s__COMMONMEM
        .globl l__COMMONMEM
        .globl s__COMMONDATA
        .globl l__COMMONDATA

        .globl kstack_top

        ; startup code
        .area _CODE
init:
        di
        ld sp, #kstack_top

        ; Configure memory map
        call init_early

	; move the common memory where it belongs    
	ld hl, #s__DATA
	ld de, #s__COMMONMEM
	ld bc, #l__COMMONMEM
	ldir
	ld de, #s__COMMONDATA
	ld bc, #l__COMMONDATA
	ldir
	; font
	ld de, #s__FONT
	ld bc, #l__FONT
	ldir
	; and the discard but do it backwards
	ld de, #s__DISCARD
	ld bc, #l__DISCARD-1
	add hl,bc
	ex de,hl
	add hl,bc
	ex de,hl
	inc bc
	lddr
	; then zero the data area
	ld hl, #s__DATA
	ld de, #s__DATA + 1
	ld bc, #l__DATA - 1
	ld (hl), #0
	ldir
	; Zero buffers area
	ld hl, #s__BUFFERS
	ld de, #s__BUFFERS + 1
	ld bc, #l__BUFFERS - 1
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

