		; Order the code into the high 32K
	        .area _CODE
	        .area _CODE1
	        .area _CODE2
		.area _VIDEO
	        .area _CONST
	        .area _DATA
	        .area _BSEG
	        .area _BSS
	        .area _HEAP
	        ; note that areas below here may be overwritten by the heap at runtime, so
	        ; put initialisation stuff in here
	        .area _GSINIT
	        .area _GSFINAL
		; Low 16K
		.area _BOOT
	        .area _INITIALIZED
	        .area _COMMONMEM
		.area _BUFFERS
		; We want the DISCARD area last as we eventually want to
		; expand all over it for buffers
		.area _DISCARD
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

                .include "kernel.def"
                .include "../kernel.def"

	        ; startup code
	        .area _BOOT

;
;		On entry the bootloader has put the banker into the
;		kernel map and loaded us at 0x100 (it's at 0x0)
;
start:
		ld sp, #kstack_top
		; Zero the data area
		ld hl, #s__DATA
		ld de, #s__DATA + 1
		ld bc, #l__DATA - 1
		ld (hl), #0
		ldir
		call init_early
		call init_hardware
		call _vtinit
		call _fuzix_main
		di
stop:		halt
		jr stop

;
; Buffers (we use asm to set this up as we need them in a special segment
; so we can recover the discard memory into the buffer pool
;

	    .globl _bufpool
	    .area _BUFFERS

_bufpool:
	    .ds BUFSIZE * NBUFS

