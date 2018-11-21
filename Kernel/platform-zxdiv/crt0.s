        .module crt0
	;
	;	Our common lives low
	;
        .area _COMMONMEM
	.area _STUBS
        .area _CONST
        .area _INITIALIZER
	;
	;	The writeables cannot start until 0x2000 but for simplicity
	;	we just start at 0x2000 for now, otherwise we have to fight
	;	the crappy loaders
	;
	.area _COMMONDATA
        .area _INITIALIZED
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
	.area _FONT
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
	.globl l__DATA
	.globl s__DATA
        .globl kstack_top

        .globl unix_syscall_entry
        .globl nmi_handler
        .globl interrupt_handler

	.include "kernel.def"

        ; startup code
	;
	; This is all a bit weird. In order to stop fatware and friends
	; screwing life up we load our lowest 8K as the boot.bin on a
	; FAT volume. It's a horrible hack but will do for the moment.
	; as it ensures we can get the low 8K correct. Otherwise the
	; standard firmware locks it and we are screwed.
	;
	; On entry therefore the low 8K is correctly valid and has loaded
	; the kernel image for us from blocks 1+ and all is good.
	;

        .area _CODE

	.globl _go

_go:

        di

	;  We need to wipe the BSS but the rest of the job is done.

	ld hl, #s__DATA
	ld de, #s__DATA+1
	ld bc, #l__DATA-1
	ld (hl), #0
	ldir

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

	; Boot marker at 0x2200

	.area _COMMONMEM
	.globl _marker
_marker:
	.byte 'Z'		; marker
	.byte 'B'
	.word _go		; boot addresss

	.area _STUBS
stubs:
	.ds 768
