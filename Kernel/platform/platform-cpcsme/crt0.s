; 2013-12-18 William R Sowerbutts

        .module crt0

        ; Ordering of segments for the linker.
        ; WRS: Note we list all our segments here, even though
        ; we don't use them all, because their ordering is set
        ; when they are first seen.
        .area _CODE
        .area _CODE2
	.area _VIDEO
        .area _CONST
        .area _INITIALIZED
	.area _INTDATA
        .area _DATA
        .area _BSEG
        .area _BSS
        .area _HEAP
        ; note that areas below here may be overwritten by the heap at runtime, so
        ; put initialisation stuff in here
        .area _GSINIT
        .area _GSFINAL
	.area _BUFFERS
	.area _DISCARD
        .area _INITIALIZER
        .area _COMMONMEM

        ; imported symbols
        .globl _fuzix_main
        .globl init_early
        .globl init_hardware
        .globl s__INITIALIZER
        .globl s__COMMONMEM
        .globl l__COMMONMEM
        .globl s__DISCARD
        .globl l__DISCARD
        .globl s__DATA
        .globl l__DATA
        .globl kstack_top
        .globl MMR_for_this_bank

	; For the boot vector
	.globl init

	.include "kernel.def"
	.include "../../cpu-z80/kernel-z80.def"


        ; startup code
        .area _CODE
init:
        di
        ld bc, #0x7fae ;RMR ->UROM disable LROM disable
        out (c),c
        ld bc, #0x7fc2 	;MMR ->Kernel map
        out (c),c
        ld sp, #kstack_top

        ; move the common memory where it belongs in kernel bank

	ld hl, #s__DATA
	ld de, #s__COMMONMEM
	ld bc, #l__COMMONMEM
	ldir

	; copy the discard to the kernel

	ld de, #s__DISCARD
	ld bc, #l__DISCARD
	ldir

        ; Configure memory map
        call init_early ;this also copies common to all banks
        ld a,#0xc2
	ld (#MMR_for_this_bank),a ; mark kernel bank with its MMR value


	; then zero the data area
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


	.area _BUFFERS
;
; Buffers (we use asm to set this up as we need them in a special segment
; so we can recover the discard memory into the buffer pool
;

	.globl _bufpool
	.area _BUFFERS

_bufpool:
	.ds BUFSIZE * NBUFS
