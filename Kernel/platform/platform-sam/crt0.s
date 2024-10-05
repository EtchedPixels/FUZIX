		; For 32K banking we need to keep anything that is common
		; and anything that might be copied to or from user space
		; in the top 32K. So fill the bottom with code
	        .area _CODE
	        .area _CODE2
		.area _HOME
		; Must be above the 32K boundary
		; FIXME: we really want to bank the fonts in the video area
		.area _VIDEO
		.area _HIGH
		.area _FONT
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
		.area _DISCARD
	        .area _INITIALIZER
	        .area _COMMONMEM
	        .area _COMMONDATA

		.area _PAGE0
		.area _PAGEH

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

		; export the buffer
		.globl _low_bounce;

	        ; startup code
	        .area _CODE

;
;	Once the loader completes it jumps here
;	Our low bank is set, our high bank is still setup
;
start:
		; Map the kernel high bank (2/3)
		ld a,#2
		out (251),a
		ld sp, #kstack_top
		; Zero the data area (shouldn't be needed)
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

;
;	Low memory bounce buffer/scratch stack space. Define it here so we
;	are sure it's below the 32K line.
;
_low_bounce:
		.ds 512
