	        ; Ordering of segments for the linker.
	        ; WRS: Note we list all our segments here, even though
	        ; we don't use them all, because their ordering is set
	        ; when they are first seen.	
	        .area _CODE
	        .area _CODE1
	        .area _CODE2
		.area _DISCARD2
		.area _VIDEO
	        .area _COMMONMEM
		.area _STUBS
	        .area _CONST
	        .area _INITIALIZED
	        .area _BSEG
	        .area _BSS
	        .area _HEAP
	        ; note that areas below here may be overwritten by the heap at runtime, so
	        ; put initialisation stuff in here
	        .area _GSINIT
	        .area _GSFINAL
	        .area _DATA
		.area _BUFFERS
	        .area _INITIALIZER
		; Buffers must be directly before discard as they will
		; expand over it

        	; imported symbols
        	.globl _fuzix_main
	        .globl init_early
	        .globl init_hardware
	        .globl s__DATA
	        .globl l__DATA
	        .globl s__BUFFERS
	        .globl l__BUFFERS
	        .globl s__COMMONMEM
	        .globl l__COMMONMEM
		.globl s__INITIALIZER
	        .globl kstack_top
		.globl bufend

		; exports
		.globl _discard_size

	        ; startup code
	        .area _BOOT

;
;	Once the loader completes it jumps here
;
start:
		ld sp, #kstack_top
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
		ld hl,#0x8000
		ld de,#bufend
		or a
		sbc hl,de
		ld (_discard_size),hl
		call init_early
		call init_hardware
		push af
		call _fuzix_main
		pop af
		di
stop:		halt
		jr stop

		.area _DATA
_discard_size:
		.dw 0

	.area _STUBS
stubs:
	.ds 768
