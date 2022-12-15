        .module crt0

        ; Ordering of segments for the linker.
        ; WRS: Note we list all our segments here, even though
        ; we don't use them all, because their ordering is set
        ; when they are first seen.

	; Start with the ROM area from C000
        .area _CODE
	.area _CODE1
        .area _CODE2
        .area _CODE3
	.area _VIDEO
	.area _CODE4
	.area _CODE5
	; Discard is loaded where process memory wil blow it away
	; Writeable memory segments
        .area _COMMONMEM
	.area _COMMONDATA
        .area _HOME     ; compiler stores __mullong etc in here if you use them
	.area _STUBS
        .area _CONST
        .area _INITIALIZED
        .area _DATA
        .area _BSEG
        .area _BSS
        .area _HEAP
        ; note that areas below here may be overwritten by the heap at runtime, so
        ; put initialisation stuff in here
        .area _BUFFERS     ; _BUFFERS grows to consume all before it (up to KERNTOP)
	; These get overwritten and don't matter
        .area _GSINIT      ; unused
        .area _GSFINAL     ; unused
        .area _DISCARD
        .area _INITIALIZER ; binman copies this to the right place for us

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

	.globl interrupt_handler
	.globl nmi_handler

	.include "kernel.def"

	; Starts at 0x4000

	.area _CODE

init:  
        di
	xor	a
	ld	sp,#kstack_top

	out	(6),a		; C000-E7FF RAM off for now TODO
	out	(0xFF),a	; Get into a known state

	; Clear the screen
	ld	a,#0x10
	out	(0x88),a
	ld	hl,#0xE800
	ld	de,#0xE801
	ld	bc,#0x03BF
	ld	(hl),#0x02
	ldir
	ld	hl,#0xEC00
	ld	de,#0xEC01
	ld	bc,#0x03BF
	ld	(hl),#' '
	ldir

	ld	a,#0xC7
	out	(0x83),a
	ld	a,#0x08		;	8 / second
	out	(0x83),a
	ld	a,#0xFB
	ld	(0xEFF8),a
	ld	hl,#0x4DED	;	reti
	ld	(0xEFF9),hl

        ; Hardware setup
        call init_hardware

        ; Call the C main routine
        call _fuzix_main
    
        ; fuzix_main() shouldn't return, but if it does...
        di
stop:   halt
        jr stop

	.area _STUBS

	.ds	600
