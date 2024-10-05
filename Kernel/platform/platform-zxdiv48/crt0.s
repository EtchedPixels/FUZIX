        .module crt0
	;
	;	Our common lives low
	;
        .area _CONST
        .area _COMMONMEM
	.area _STUBS
	.area _COMMONDATA
        .area _INITIALIZED
	;
	;	Beyond this point we just zero.
	;
        .area _DATA
        .area _BSEG
        .area _BSS
        .area _HEAP
        .area _GSINIT
        .area _GSFINAL
	;
	;	All our code is banked at 0x0080 (with stubs at 0-7F)
	;
        .area _CODE1
	.area _CODE2
	;	We start this bank with FONT so that we have it aligned
	.area _FONT
	.area _CODE3
	.area _DATA3
        .area _VIDEO
	.area _CODE4

	; Discard is dumped in at 0xA000 and will be blown away later.
        .area _DISCARD
	; Somewhere to throw it out of the way
        .area _INITIALIZER

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
	.include "../../cpu-z80/kernel-z80.def"

	;
        ; startup code. Called with most stuff indeterminate but CODE1
        ; mapped. On entry A is the machine type.
	; A = 0		DivIDE plus
	; A = 1		ZXCF 512K+ (we need to do sizing later)
	;

        .area _CODE1

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

	.area _COMMONDATA
	.globl _marker
_marker:
	.byte 'Z'		; marker
	.byte 'B'
	.word _go		; boot addresss

	.area _STUBS
stubs:
	.ds	540

	.area _DATA3
;
;	Buffers live in segment 3 where we have a little bit of room
;

	.globl _bufdata

_bufdata:
	.ds 512 * NBUFS

