        .module crt0

        .area _CODE
	.area _CODE2
	.area _CODE3
        .area _VIDEO
        .area _INITIALIZED
        .area _HOME
	.area _CONST

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
        .area _INITIALIZER

        .area _DISCARD
	; Somewhere to throw it out of the way


	; Our common is mapped high
	.area _COMMONDATA
        .area _COMMONMEM
	.area _FONT



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

	;
        ; startup code. Runs from 0x100. The kernel is mapped into pages
	; 0/1/2/3 but 0 / 5 / 2 / 3 are currently mapped. Stack is not
	; necessarily valid on entry, shadow mode is on and interrupts are
	; off.
	;

        .area _CODE

	.globl _start

_start:
	; Map in the other 16K
	out (c),a

	; Black border so we know we arrived
	xor a
	out (c),a

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
        call init_early

        ; Hardware setup
        call init_hardware

        ; Call the C main routine
        call _fuzix_main
    
        ; main shouldn't return, but if it does...
        di
stop:   halt
        jr stop

	.area _BUFFERS
;
; Buffers (we use asm to set this up as we need them in a special segment
; so we can recover the discard memory into the buffer pool
;

	.globl _bufpool
	.area _BUFFERS

_bufpool:
	.ds BUFSIZE * NBUFS
