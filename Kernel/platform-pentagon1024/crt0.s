        .module crt0
	;
	;	Our common lives low
	;
	.area _COMMONDATA
        .area _COMMONMEM
        .area _CONST
	.area _STUBS
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
	;	Finally the buffers so they can expand
	;
	.area _BUFFERS
	;
	;	All our code is banked at 0xC000
	;
        .area _CODE1
	.area _CODE2
	;	We start this bank with FONT so that we have it aligned
	.area _FONT
	.area _CODE3
        .area _VIDEO

	; Discard is dumped in at 0x8000 and will be blown away later.
        .area _DISCARD
	; Somewhere to throw it out of the way
        .area _INITIALIZER

        ; imported symbols
        .globl _fuzix_main
        .globl init_early
        .globl init_hardware
	.globl l__BUFFERS
	.globl s__BUFFERS
	.globl l__DATA
	.globl s__DATA
        .globl kstack_top

        .globl unix_syscall_entry
        .globl nmi_handler
        .globl interrupt_handler

	.include "kernel.def"
	.include "../kernel-z80.def"

	.area _CODE1
        ; startup code
	;
	; Entered from the bootstrap with 0-3FFF mapped and loaded as
	; RAM bank 0
	;
        di

	;  We need to wipe the BSS but the rest of the job is done.

	ld hl, #s__DATA
	ld de, #s__DATA+1
	ld bc, #l__DATA-1
	ld (hl), #0
	ldir
	ld hl, #s__BUFFERS
	ld de, #s__BUFFERS+1
	ld bc, #l__BUFFERS-1
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

	.area _COMMONDATA

	.area _STUBS
stubs:
	.ds 600

	.area _BUFFERS
;
; Buffers (we use asm to set this up as we need them in a special segment
; so we can recover the discard memory into the buffer pool
;

	.globl _bufpool
	.area _BUFFERS

_bufpool:
	.ds BUFSIZE * NBUFS
