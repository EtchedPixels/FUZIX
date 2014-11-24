; 2013-12-18 William R Sowerbutts

        .module crt0

	.include "pages.def"
	
        ; Ordering of segments for the linker.
        ; WRS: Note we list all our segments here, even though
        ; we don't use them all, because their ordering is set
        ; when they are first seen.
        .area _CODE
        .area _COMMONMEM
        .area _DATA
        .area _CODE2
        .area _VIDEO
        .area _DISCARD      ; not discarded yet
        .area _CONST
        .area _FONT
        .area _INITIALIZED
        .area _BSEG
        .area _BSS
        .area _HEAP
        ; note that areas below here may be overwritten by the heap at runtime, so
        ; put initialisation stuff in here
        .area _INITIALIZER
        .area _GSINIT
        .area _GSFINAL

        ; imported symbols
        .globl _fuzix_main
        .globl init_early
        .globl init_hardware
        .globl s__INITIALIZER
        .globl l__INITIALIZER
        .globl s__COMMONMEM
        .globl l__COMMONMEM
        .globl s__INITIALIZED
        .globl l__INITIALIZED
        .globl s__DATA
        .globl l__DATA
        .globl s__BSS
        .globl l__BSS
        .globl kstack_top


        .globl unix_syscall_entry
        .globl nmi_handler
        .globl interrupt_handler

	;
        ; Startup code
        ;
        .area _CODE

; Interrupt vector tables

;       rst 0
init:	jp .+3			; // *((char*)0) == 0xC3
        di
        jp init_continue
rst0e:

;       rst 8
	.ds 8 - (rst0e-init)
	reti
rst8e:

;       rst 10
	.ds 0x10 - (rst8e-init)
	reti
rst10e:

;       rst 18
	.ds 0x18 - (rst10e-init)
	reti
rst18e:

;       rst 20
	.ds 0x20 - (rst18e-init)
	reti
rst20e:

;       rst 28
	.ds 0x28 - (rst20e-init)
	reti
rst28e:

;---    rst 30 syscall entry
	.ds 0x30 - (rst28e-init)
        jp unix_syscall_entry
rst30e:

;---    rst 38 interrupt handler (timer 20ms)
	.ds 0x38 - (rst30e-init)
	jp interrupt_handler
rst38e:

;---    rst 66 NMI handler
	.ds 0x66 - (rst38e-init)
        jp nmi_handler


; CONTINUE STARTUP CODE
init_continue:
        ld sp, #kstack_top

        ; Configure memory map
        call init_early

	; move the common memory where it belongs
;        ld hl, #s__INITIALIZER
;        ld de, #s__COMMONMEM
;        ld bc, #l__COMMONMEM
;        ldir

	; then zero the data area
	ld hl, #s__DATA
        ld de, #s__DATA + 1
        ld bc, #l__DATA - 1
        ld (hl), #0
        ldir

        ; Hardware setup
        call init_hardware

        ; Call the C main routine
        call _fuzix_main
    
        ; main shouldn't return, but if it does...
        di
stop:   halt
        jr stop

