	        ; Ordering of segments for the linker.
	        ; WRS: Note we list all our segments here, even though
	        ; we don't use them all, because their ordering is set
	        ; when they are first seen.	
	        .area _CODE
	        .area _CODE1
	        .area _CODE2
		.area _VIDEO
		.area _DATA2
		; We want the DISCARD2 area last as we eventually want to
		; expand all over it for buffers
		.area _BUFFERS2
		.area _DISCARD
		.area _DISCARD2
		.area _UDATA
		.area _BOOT
		.area _COMMONDATA
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
	        .area _INITIALIZER
		; Buffers must be directly before discard as they will
		; expand over it

        	; imported symbols
        	.globl _fuzix_main
	        .globl init_early
	        .globl init_hardware
		.globl _vtinit
	        .globl s__DATA
	        .globl l__DATA
	        .globl s__COMMONMEM
	        .globl l__COMMONMEM
		.globl s__INITIALIZER
	        .globl kstack_top

	        ; startup code
	        .area _BOOT

;
;	Once the loader completes it jumps here
;	A holds the type of mapper the boot block was for
;
;	0: Supermem
;	1: Selector
;
start:
		; Take care to preserve A until init_early
		ld sp, #kstack_top	; just below us
		; then zero the data area
		ld hl, #s__DATA
		ld de, #s__DATA + 1
		ld bc, #l__DATA - 1
		ld (hl), #0
		ldir
		; We pass A into init_early holding the mapper type
		call init_early
		call init_hardware
		push af
		call _vtinit
		pop af
		push af
		call _fuzix_main
		pop af
		di
stop:		halt
		jr stop


		.area _STUBS
stubs:
		.ds 768
