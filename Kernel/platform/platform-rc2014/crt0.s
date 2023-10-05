; 2015-02-20 Sergey Kiselev
; 2013-12-18 William R Sowerbutts

        .module crt0

        ; Ordering of segments for the linker.
        ; WRS: Note we list all our segments here, even though
        ; we don't use them all, because their ordering is set
        ; when they are first seen.
	.area _CODE
        .area _HOME     ; compiler stores __mullong etc in here if you use them
	.area _STUBS
        .area _CONST
        .area _INITIALIZED
        .area _DATA
        .area _BSEG
        .area _BSS
        .area _HEAP
        .area _GSINIT      ; unused
        .area _GSFINAL     ; unused
	.area _BUFFERS
        .area _DISCARD
	.area _FONT	   ; only used at start up so discardable
	.area _CODE1
	.area _DISCARD1
        .area _CODE2
	.area _DISCARD2
	.area _VECTORS	    ; 32 byte aligned
        .area _COMMONMEM
	.area _COMMONDATA
        .area _INITIALIZER ; binman copies this to the right place for us

        ; exported symbols
        .globl init

        ; imported symbols
        .globl _fuzix_main
        .globl init_hardware
        .globl s__INITIALIZER
        .globl s__COMMONMEM
        .globl l__COMMONMEM
        .globl s__DISCARD
        .globl l__DISCARD
        .globl s__BUFFERS
        .globl l__BUFFERS
        .globl s__DATA
        .globl l__DATA
        .globl kstack_top

	.globl _systype
	.globl _syskhz
	.globl _syscpu

	.include "kernel.def"
	.include "../../cpu-z80/kernel-z80.def"

        ; startup code. Starts _CODE because we want it to be at 0x0100
	; Entered with SP high and the stack holding platform info
	; from the ROM

        .area _CODE
        ; must be at 0x0100 as we are loaded at that
init:
	; Get the ROM info off the stack before we flip banks about
	pop bc			; B = ROMWBW ver C = platform
	pop de			; DE = CPU speed in KHz
	pop hl			; H - Z80 variant L = CPU speed in MHz

	; setup the memory paging for kernel
        ld a, #33
        out (MPGSEL_1), a       ; map page 33 at 0x4000
        inc a
        out (MPGSEL_2), a       ; map page 34 at 0x8000
	inc a
        out (MPGSEL_3), a       ; map page 35 at 0xC000

mappedok:
        ; switch to stack in high memory
        ld sp, #kstack_top

	; Save the ROM info
	exx
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

	exx
	; We now have the ROM info to hand and have cleared stuff so it
	; is safe to write these into memory

	ld (_systype), bc
	ld (_syskhz), de
	ld (_syscpu), hl

        ; Hardware setup
	push af
        call init_hardware
	pop af

        ; Call the C main routine
	push af
        call _fuzix_main
	pop af
    
        ; fuzix_main() shouldn't return, but if it does...
        di
stop:   halt
        jr stop

;
;	Call stubs are filled in here by the linker
;

	.area _STUBS
	.ds 600

	.area _BUFFERS
;
; Buffers (we use asm to set this up as we need them in a special segment
; so we can grow to fit the space free
;
	.globl _bufpool
	.area _BUFFERS

_bufpool:
	.ds BUFSIZE * NBUFS
