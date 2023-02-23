
        .module crt0

        ; Ordering of segments for the linker.
        ; WRS: Note we list all our segments here, even though
        ; we don't use them all, because their ordering is set
        ; when they are first seen.
        .area	_CODE
        .area	_HOME     ; compiler stores __mullong etc in here if you use them
        .area	 _CODE2
        .area	_CONST
        .area	_DATA
        .area	_INITIALIZED
        .area	 _BSEG
        .area	_BSS
        .area	_HEAP
        ; note that areas below here may be overwritten by the heap at runtime, so
        ; put initialisation stuff in here
        .area	_GSINIT      ; unused
        .area	_GSFINAL     ; unused
        .area	_DISCARD
        .area	_INITIALIZER
        .area	_COMMONMEM
	.area	_SERIALDATA
	.area	_SERIAL

        ; exported symbols
        .globl	init

	; Alignment check
	.globl	vectors
	.globl	nmi
	.globl	rst38

        ; imported symbols
        .globl	_fuzix_main
        .globl	init_hardware
	.globl	interrupt_handler
        .globl	s__INITIALIZER
        .globl	s__COMMONMEM
        .globl	l__COMMONMEM
        .globl	s__DISCARD
        .globl	l__DISCARD
        .globl	s__DATA
        .globl	l__DATA
        .globl	kstack_top

	; interrupt vectors
	.globl spurious
	.globl siob_txd
	.globl siob_status
	.globl siob_rx_ring
	.globl siob_special
	.globl sioa_txd
	.globl sioa_status
	.globl sioa_rx_ring
	.globl sioa_special

	.globl	nmi_handler

	.include "kernel.def"

	; Dummy page0 area so binman doesn't pack us

	.area _PAGE0

        ; startup code

	; We run from ROM rather than fight the existing ROM stuff
	; for now. Maybe when it gets RomWBW we might change the plan

        .area _CODE
init:                       ; must be at 0x0100 as we are loaded at that
	di
	ld	a,#0x21
	out	(MPGSEL_3),a	; Map RAM in the top 16K
	jp	boot
rst8:	ret
	.ds	7
rst10:	ret
	.ds	7
rst18:	ret
	.ds	7
rst20:	ret
	.ds	7
rst28:	ret
	.ds	7
rst30:	ret
	.ds	7
rst38:	jp	interrupt_handler	; For now TODO - use IM2
	.ds	5
im2_table:				; at 0x40
	.ds	0x26			; to 0x66 for NMI
nmi:	jp	nmi_handler
	.ds	23
vectors:
; 0x80: Our IM2 table
	.word	spurious
	.word	spurious
	.word	spurious
	.word	interrupt_handler	; CTC 3 - timer tick
	.word	0
	.word	0
	.word	0
	.word	0
; 0x90 SIO
	.word	siob_txd
	.word	siob_status
	.word	siob_rx_ring
	.word	siob_special
	.word	sioa_txd
	.word	sioa_status
	.word	sioa_rx_ring
	.word	sioa_special
; 0xA0 onward unused

boot:
	xor	a
	out	(MPGSEL_0),a	; identity map
	inc	a
	out	(MPGSEL_1),a
	;	MMU on
	out	(MPGENA),a
	;	Map stuff to copy the RAM 16K block
	ld	a,#0x03
	out	(MPGSEL_2),a
	ld	hl,#0x8000
	ld	de,#0xC000
	ld	bc,#0x4000
	ldir

	ld	a,#0x20
	out	(MPGSEL_2),a
	ld	a,#0x02
	out	(MPGSEL_3),a
	ld	hl,#0xC000
	ld	de,#0x8000
	ld	bc,#0x4000
	ldir

	;	Correct the mapping to 0/1/20/21
	ld	a,#0x21
	out	(MPGSEL_3),a

        ; switch to stack in high memory
        ld sp, #kstack_top

        ; Zero the data area
        ld hl, #s__DATA
        ld de, #s__DATA + 1
        ld bc, #l__DATA - 1
        ld (hl), #0
        ldir

        ; Hardware setup
        call init_hardware

        ; Call the C main routine
        call _fuzix_main
    
        ; fuzix_main() shouldn't return, but if it does...
        di
stop:   halt
        jr stop
