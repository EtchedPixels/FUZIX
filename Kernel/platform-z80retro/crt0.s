
        .module crt0

        ; Ordering of segments for the linker.
        ; WRS: Note we list all our segments here, even though
        ; we don't use them all, because their ordering is set
        ; when they are first seen.
        .area	_CODE
        .area	 _CODE2
        .area	_HOME     ; compiler stores __mullong etc in here if you use them
        .area	_CONST
        .area	_INITIALIZED
        .area	_DATA
        .area	 _BSEG
        .area	_BSS
        .area	_HEAP
        ; note that areas below here may be overwritten by the heap at runtime, so
        ; put initialisation stuff in here
        .area	_GSINIT      ; unused
        .area	_GSFINAL     ; unused
        .area	_DISCARD
        .area	_INITIALIZER
        .area	_COMMONMEM
        .area	_COMMONDATA
	.area	_SERIALDATA
	.area	_SERIAL

        ; exported symbols
        .globl	init

        ; imported symbols
        .globl	_fuzix_main
        .globl	init_hardware
	.globl	interrupt_handler
        .globl s__DATA
        .globl l__DATA
        .globl s__DISCARD
        .globl l__DISCARD
        .globl s__COMMONMEM
        .globl l__COMMONMEM
        .globl s__COMMONDATA
        .globl l__COMMONDATA
	.globl s__INITIALIZER
        .globl kstack_top
        .globl	kstack_top

	.include "kernel.def"

        ; startup code

	; We run from ROM rather than fight the existing ROM stuff
	; for now. Maybe when it gets RomWBW we might change the plan

        .area _CODE
init:                       ; must be at 0x0100 as we are loaded at that
	di

	ld	sp, #kstack_top
	; Move the common memory where it belongs
	ld	hl, #s__DATA
	ld	de, #s__COMMONMEM
	ld	bc, #l__COMMONMEM
	ldir
;	ld	de, #s__COMMONDATA
;	ld	bc, #l__COMMONDATA
;	ldir
	; then the discard
	; Discard can just be linked in but is next to the buffers
	ld	de, #s__DISCARD
	ld	bc, #l__DISCARD
	ldir
	; then zero the data area
	ld	hl, #s__DATA
	ld	de, #s__DATA + 1
	ld	bc, #l__DATA - 1
	ld	(hl), #0
	ldir
	; Zero buffers area
;	ld	hl, #s__BUFFERS
;	ld	de, #s__BUFFERS + 1
;	ld	bc, #l__BUFFERS - 1
;	ld	(hl), #0
;	ldir

        ; Hardware setup
        call	init_hardware

        ; Call the C main routine
        call	_fuzix_main
    
        ; fuzix_main() shouldn't return, but if it does...
        di
stop:   halt
        jr stop
