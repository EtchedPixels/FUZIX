        .module crt0
	;	The code part of the setup from 0x4000
	.area _CODE
        .area _CODE1
	.area _CODE2
	.area _CODE3
	;
	;	Beyond this point we just zero.
	;
        .area _BSS
        .area _HEAP
        .area _GSINIT
        .area _GSFINAL

	.area _BUFFERS
        .area _DISCARD
	.area _PAGE0		; tell tools not to pack us

	;
	;	Our common lives low
	;
	;	We start this bank with FONT so that we have it aligned
	;
	;	0x0000-0x1FFF is read only and we can't use 1FF8-1FFF for
	;	code
	;
	.area _FONTCOMMON
        .area _VIDEO
        .area _COMMONMEM
        .area _CONST
        .area _INITIALIZER

	;	0x2000-0x3FFF - avoid code in 3D00-3DFF
	.area _COMMONDATA
        .area _INITIALIZED
        .area _DATA
        .area _BSEG


        ; imported symbols
        .globl _fuzix_main
        .globl init_early
        .globl init_hardware
	.globl l__BUFFERS
	.globl s__BUFFERS
	.globl l__DATA
	.globl s__DATA
        .globl kstack_top

        .globl nmi_handler
	.globl null_handler
        .globl interrupt_handler
	.globl ___sdcc_enter_ix

	.include "kernel.def"
	.include "../../cpu-z80/kernel-z80.def"


;	We are run from 0x0000 (_CODE1)
;	On entry we have main memory mapped, DIVMMC bank 0 mapped over
;	the ROM space and the top 16K is an undefined bank. SP is not
;	valid, interrupts are off.
;

        .area _BOOT(ABS)

	.org	0

	.globl go

	; This first chunk of this gets overwritten by vectors

rst0:
	jp	null_handler
	jp	go
	nop
	nop
rst8:
	jp	___sdcc_enter_ix
	.ds	5
rst10:
	ld	sp,ix
	pop	ix
	ret
	.ds	3
rst18:
	pop	af
	pop	ix
	ret
	.ds	4
rst20:	ld	a,(hl)
	inc	hl
	ld	h,(hl)
	ld	l,a
	ret
	.ds	3
rst28:	ret
	.ds	7
rst30:	ret
	.ds	7
rst38:	jp	interrupt_handler
	.ds	5
	.ds	0x26
	jp	nmi_handler

go:
	; On entry we are in ZX128 made we have the mapping as
	; ROM/5/2/? and screen in 7. (5/8/? on systens with bank 2 not
	; shared..
	; Wipe screen with test pattern
	ld	bc,#0x7ffd
	ld	a,#0x0F
	out	(c),a
	ld	hl,#0xC000
	ld	de,#0xC001
	ld	bc,#0x3FFF
	ld	(hl),#0xAA
	ldir

	; We are living in the DIVMMC mapping and we now need to fix the
	; top mapping to be 3 with screen in 7
	ld	bc,#0x7ffd
	ld	a,#0x0B
	out	(c),a

	; Hires mode
	ld	a,#0x3E
	out	(0xFF),a

	;  We need to wipe the BSS but the rest of the job is done.

	ld	hl, #s__DATA
	ld	de, #s__DATA+1
	ld	bc, #l__DATA-1
	ld	(hl), #0
	ldir
	ld	hl, #s__BUFFERS
	ld	de, #s__BUFFERS+1
	ld	bc, #l__BUFFERS-1
	ld	(hl), #0
	ldir

	jp	boot

	.area	_CODE1
boot:

        ld	sp, #kstack_top

        ; Configure memory map
	push	af
        call	init_early
	pop	af

        ; Hardware setup
	push	af
        call	init_hardware
	pop	af

        ; Call the C main routine
	push	af
        call	_fuzix_main
	pop	af
    
        ; main shouldn't return, but if it does...
        di
stop:   halt
        jr	stop

	.area _BUFFERS
;
; Buffers (we use asm to set this up as we need them in a special segment
; so we can recover the discard memory into the buffer pool
;

	.globl _bufpool
	.area _BUFFERS

_bufpool:
	.ds	BUFSIZE * NBUFS
