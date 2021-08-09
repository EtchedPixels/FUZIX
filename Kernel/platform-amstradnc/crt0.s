	        ; Ordering of segments for the linker.
	        ; WRS: Note we list all our segments here, even though
	        ; we don't use them all, because their ordering is set
	        ; when they are first seen.	
	        .area _CODE
	        .area _CODE2
		.area _HOME
		.area _FONT
		.area _VIDEO
	        .area _CONST
	        .area _INITIALIZED
	        .area _DATA
	        .area _BSEG
	        .area _BSS
	        .area _HEAP
	        .area _INITIALIZER
	        ; note that areas below here may be overwritten by the heap at runtime, so
	        ; put initialisation stuff in here
	        .area _GSINIT
	        .area _GSFINAL
	        .area _COMMONMEM
		.area _DISCARD

        	; imported symbols
        	.globl _fuzix_main
	        .globl init_early
	        .globl init_hardware
	        .globl s__DATA
	        .globl l__DATA
	        .globl s__DISCARD
	        .globl l__DISCARD
	        .globl s__COMMONMEM
	        .globl l__COMMONMEM
		.globl s__INITIALIZER
	        .globl kstack_top

	        ; startup code @0
	        .area _CODE

; This area will be covered by the boot block in the boot image and then
; copid into the runtime one. We reuse the low 0x80 for vectors
		.ds	#0x223
;
; Execution begins with us correctly mapped and at 0x0223
;
;
;
;	Our copy of the image is entered here with the system mapped at
; banks 3,4,5,6 (0-2 hold the bootblock and image for rebooting)
;
start:		di
		ld sp, #kstack_top
		; move the common memory where it belongs    
		ld hl, #s__DATA
		ld de, #s__COMMONMEM
		ld bc, #l__COMMONMEM
		ldir
		; and the discard
		ld de, #s__DISCARD
		ld bc, #l__DISCARD
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
