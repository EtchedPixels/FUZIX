	        ; Ordering of segments for the linker.
	        ; WRS: Note we list all our segments here, even though
	        ; we don't use them all, because their ordering is set
	        ; when they are first seen.	
	        .area _CODE
	        .area _CODE2
		.area _VIDEO
	        .area _CONST
		.area _DISCARD
	        .area _DATA
	        .area _INITIALIZED
	        .area _BSEG
	        .area _BSS
	        .area _HEAP
	        ; note that areas below here may be overwritten by the heap at runtime, so
	        ; put initialisation stuff in here
	        .area _INITIALIZER
	        .area _GSINIT
	        .area _GSFINAL
	        .area _COMMONMEM

        	; imported symbols
        	.globl _fuzix_main
	        .globl init_early
	        .globl init_hardware
	        .globl s__DATA
	        .globl l__DATA
	        .globl s__COMMONMEM
	        .globl l__COMMONMEM
		.globl s__INITIALIZER
	        .globl kstack_top

		; Just for the benefit of the map file
		.globl start

	        ; startup code @0x100
	        .area _CODE

;
; Execution begins with us correctly mapped and at 0x0x100
;
start:		di
		; Debug port
		ld a, #0x23
	        out (0x2e), a
		ld a, #'@'
		out (0x2f), a
		ld sp, #kstack_top
		; move the common memory where it belongs    
		ld hl, #s__INITIALIZER
		ld de, #s__COMMONMEM
		ld bc, #l__COMMONMEM
		ldir
		; then zero the data area
		ld hl, #s__DATA
		ld de, #s__DATA + 1
		ld bc, #l__DATA - 1
		ld (hl), #0
		ldir

		call init_early
		call init_hardware
		call _fuzix_main
		di
stop:		halt
		jr stop

