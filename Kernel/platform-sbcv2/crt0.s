	        ; Ordering of segments for the linker.
	        .area _CODE
	        .area _CODE2
		.area _HOME
	        .area _CONST
	        .area _INITIALIZED
	        .area _DATA
	        .area _BSEG
	        .area _BSS
	        .area _HEAP
	        .area _GSINIT
	        .area _GSFINAL
		.area _BUFFERS
		.area _DISCARD
	        .area _COMMONMEM
	        ; note that areas below here may be overwritten by the heap at runtime, so
	        ; put initialisation stuff in here
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
		.globl s__INITIALIZER
	        .globl kstack_top
		.globl map_kernel

	        ; startup code
	        .area _CODE

		; Load at 0x0100
		; We are executed as a CP/M task so live in bank 14/15
		; with CP/M under us and the HBIOS proxy at FE00
start:
		ld bc,#0x0100
		ld e,#'*'
	        rst 8
		di
		ld sp, #kstack_top
		; move the common memory where it belongs    
		ld hl, #s__DATA
		ld de, #s__COMMONMEM
		ld bc, #l__COMMONMEM
		ldir
		; then the discard
		; Discard can just be linked in but is next to the buffers
		ld de, #s__DISCARD
		ld bc, #l__DISCARD
		ldir
		ld bc,#0x0100
		ld e,#':'
	        rst 8
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
		ld bc,#0x0100
		ld e,#'X'
	        rst 8
		call init_early
		ld bc,#0x0100
		ld e,#'Y'
	        rst 8
		call init_hardware
		ld bc,#0x0100
		ld e,#'Z'
	        rst 8
		call _fuzix_main
		di
stop:		halt
		jr stop
