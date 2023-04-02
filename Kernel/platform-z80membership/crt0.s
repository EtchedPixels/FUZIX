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
		; Buffers will expand over the other junk
		.area _BUFFERS
	        .area _INITIALIZER
		.area _DISCARD


	        .area _COMMONMEM
	        .area _COMMONDATA

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
		.globl map_kernel

		; This lands high so we can boot nicely

	        ; startup code
	        .area _CODE

start:
		di
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
		call init_early
		call init_hardware
		call _fuzix_main
		di
stop:		halt
		jr stop
