;
;	We are loaded from CP/M at the moment
;
	        .module crt0

	        ; Ordering of segments for the linker.
	        ; WRS: Note we list all our segments here, even though
	        ; we don't use them all, because their ordering is set
	        ; when they are first seen.


	        .area _CODE
	        .area _CODE2
	        .area _HOME     ; compiler stores __mullong etc in here if you use them
	        .area _CONST
	        .area _INITIALIZED
	        .area _DATA
	        .area _BSEG
	        .area _BSS
	        .area _GSINIT      ; unused
	        .area _GSFINAL     ; unused
	        .area _HEAP
	        ; note that areas below here may be overwritten by the heap at runtime, so
	        ; put initialisation stuff in here
	        .area _BUFFERS     ; _BUFFERS grows to consume all before it (up to KERNTOP)
	        .area _DISCARD
		; These get overwritten and don't matter
	        .area _INITIALIZER ; binman copies this to the right place for us

		.area _BOOT
		.area _COMMONMEM
		.area _SERIALDATA

		.area _SERIAL

	        ; imported symbols
	        .globl _fuzix_main
	        .globl init_hardware
	        .globl s__INITIALIZER
	        .globl s__COMMONMEM
	        .globl l__COMMONMEM
	        .globl s__DISCARD
	        .globl l__DISCARD
	        .globl s__DATA
	        .globl l__DATA
	        .globl s__BUFFERS
	        .globl l__BUFFERS
	        .globl kstack_top

		.globl interrupt_handler
		.globl nmi_handler

		.include "kernel.def"

;
;	We don't want our image packed
;
		.area _PAGE0
;
;	Runs from 0x0100
;
		.area _BOOT

		di

		ld sp, #kstack_top
		; Zero the data area
		ld hl, #s__DATA
		ld de, #s__DATA + 1
		ld bc, #l__DATA - 1
		ld (hl), #0
		ldir
		; Zero buffers area
		ld hl, #s__BUFFERS
		ld de, #s__BUFFERS + 1
		ld bc, #l__BUFFERS - 1
		ld (hl), #0
		ldir

        	; Hardware setup
	        call init_hardware

		jp launch

		.area _CODE
launch:
	        ; Call the C main routine
	        call _fuzix_main
    
	        ; fuzix_main() shouldn't return, but if it does...
	        di
stop:		halt
		jr stop

