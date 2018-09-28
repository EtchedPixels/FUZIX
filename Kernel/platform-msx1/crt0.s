	        ; Ordering of segments for the linker.
		; ROM segments first
	        .area _CODE
		.area _LOW
		.area _HEADER
	        .area _CODE2
		.area _HOME
		.area _VIDEO
	        .area _CONST
		.area _DISCARD
		; ROM section must end with the initializer
	        .area _INITIALIZER
		; These are unpacked
	        .area _COMMONMEM
	        .area _INITIALIZED
	        .area _GSINIT
	        .area _GSFINAL
	        .area _BSEG
	        .area _BSS
	        .area _HEAP
	        .area _DATA

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
		.globl interrupt_handler

		.globl rst38		; for checking
;
;	First _CODE section
;
		.area _CODE

		.ds 0x38
rst38:		jp interrupt_handler
		; FIXME NMI etc ?
