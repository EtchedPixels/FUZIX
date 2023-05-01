
		; Code block (can be top of banked space or in motherboard memory)
	        .area _CODE
	        .area _CODE2
		.area _HOME
		.area _VIDEO
	        .area _CONST
	        .area _INITIALIZED
	        .area _DATA
	        .area _BSEG
	        .area _BSS
	        .area _HEAP
	        ; note that areas below here may be overwritten by the heap at runtime, so
	        ; put initialisation stuff in here
	        .area _GSINIT
	        .area _GSFINAL
		; Buffers must be directly before discard as they will
		; expand over it
		.area _BUFFERS
		.area _DISCARD
	        .area _COMMONMEM

	        .area _COMMONDATA
	        ; Doesn't matter if these go over the I/O space as they are
		; removed at the end of the build
	        .area _INITIALIZER


        	; imported symbols
        	.globl _fuzix_main
	        .globl init_early
	        .globl init_hardware
	        .globl s__DATA
	        .globl l__DATA
	        .globl s__DISCARD
	        .globl l__DISCARD
	        .globl s__BUFFERS
	        .globl l__BUFFERS
	        .globl s__COMMONMEM
	        .globl l__COMMONMEM
	        .globl s__COMMONDATA
	        .globl l__COMMONDATA
		.globl s__INITIALIZER
	        .globl kstack_top

		; exports
		.globl _discard_size

	        ; startup code
	        .area _CODE

;
;	We get booted from CP/M or a disk loader.
;	We are loaded at 0x0100 flat with the high things packed
;
start:
		ld sp, #kstack_top
		; move the common memory where it belongs    
		ld hl, #s__DATA
		ld de, #s__COMMONMEM
		ld bc, #l__COMMONMEM
		ldir
		ld de, #s__COMMONDATA
		ld bc, #l__COMMONDATA
		ldir
		; then the discard
; Discard can just be linked in but is next to the buffers
		ld de, #s__DISCARD
		ld bc, #l__DISCARD
		ldir
		; then zero the data area
		ld hl, #s__DATA
		ld de, #s__DATA + 1
		ld bc, #l__DATA - 1
		ld (hl), #0
		ldir
;		Zero buffers area
		ld hl, #s__BUFFERS
		ld de, #s__BUFFERS + 1
		ld bc, #l__BUFFERS - 1
		ld (hl), #0
		ldir
		ld hl,#s__COMMONMEM
		ld de,#s__DISCARD
		or a
		sbc hl,de
		ld (_discard_size),hl
		call init_early
		call init_hardware
		call _fuzix_main
		di
stop:		halt
		jr stop

		.area _DISCARD
_discard_size:
		.dw 0
