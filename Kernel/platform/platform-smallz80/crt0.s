	        ; Ordering of segments for the linker.
	        .area _CODE
	        .area _CODE2
		.area _HOME
		; Buffers will expand over the other junk
		.area _BUFFERS
	        .area _INITIALIZER
		.area _DISCARD

		.area _BOOT
	        .area _CONST
	        .area _INITIALIZED
	        .area _DATA
	        .area _BSEG
	        .area _BSS
	        .area _HEAP
	        .area _GSINIT
	        .area _GSFINAL

		; Tell the binman tool not to pack this binary
		.area _PAGE0

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

		; This lands at 0x100 so we can boot nicely

		.area _BOOT
		jp start

	        ; startup code
	        .area _CODE

start:
		di
		ld sp, #kstack_top
		; move the common memory where it belongs as
		; we packed it down to E800 for the loader
		ld hl, #0xE800
		ld de, #s__COMMONMEM
		ld bc, #l__COMMONMEM
		ldir
		ld de, #s__COMMONDATA
		ld bc, #l__COMMONDATA
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
