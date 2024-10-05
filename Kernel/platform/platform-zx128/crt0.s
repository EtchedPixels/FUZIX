        .module crt0

	;
	;	Our common and data live in 0x4000-0x7FFF
	;
	.area _COMMONDATA
        .area _COMMONMEM
	.area _STUBS
        .area _CONST
        .area _INITIALIZED
        .area _INITIALIZER
	;
	;	We move	INITIALIZER into INITIALIZED at preparation time
	;	then pack COMMONMEM.. end of INITIALIZED after DISCARD
	;	in the load image. Beyond that point we just zero.
	;
        .area _DATA
        .area _BSEG
        .area _BSS
        .area _HEAP
        .area _GSINIT
        .area _GSFINAL
	;
	;	All our code is banked at 0xC000
	;
        .area _CODE
	.area _CODE2
	;
	; Code3 sits above the display area along with the font and video
	; code so that they can access the display easily. It lives at
	; 0xDB00 therefore
	;
	.area _CODE3
        .area _VIDEO

	; FIXME: We should switch to the ROM font and an ascii remap ?
        .area _FONT
	; Discard is dumped in at 0x8000 and will be blown away later.
        .area _DISCARD

        ; imported symbols
        .globl _fuzix_main
        .globl init_early
        .globl init_hardware
        .globl l__COMMONMEM
        .globl l__STUBS
        .globl l__COMMONDATA
        .globl l__INITIALIZED
	.globl l__CONST
	.globl s__DISCARD
	.globl l__DISCARD
        .globl kstack_top

        .globl unix_syscall_entry
        .globl nmi_handler
        .globl interrupt_handler

	.include "kernel.def"

        ; startup code
        .area _CODE
init:
	jp init1	;	0xC000 - entry point
	jp init2	;	0xC003 - entry point for .sna debug hacks
init1:
        di

	;  We need to wipe the BSS etc, then copy the initialized data
	;  and common etc from where they've been stuffed above the
	;  discard segment loaded into 0x8000

	ld hl, #0x4000
	ld de, #0x4001
	ld bc, #0x3FFF
	ld (hl), #0
	ldir
	ld hl, #s__DISCARD
	ld de, #l__DISCARD
	add hl, de		; linker dumbness workarounds
	ld de, #0x4000
	ld bc, #l__COMMONMEM
	ldir
	ld bc, #l__STUBS
	ldir
	ld bc, #l__COMMONDATA
	ldir
	ld bc, #l__CONST
	ldir
	ld bc, #l__INITIALIZED
	ldir

init2:
	di
        ld sp, #kstack_top

        ; Configure memory map
	push af
        call init_early
	pop af

        ; Hardware setup
	push af
        call init_hardware
	pop af

        ; Call the C main routine
	push af
        call _fuzix_main
	pop af
    
        ; main shouldn't return, but if it does...
        di
stop:   halt
        jr stop

	.area _STUBS
stubs:
	.ds 768
