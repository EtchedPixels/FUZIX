	; imported symbols
	.import init_early
	.import init_hardware
	.import _fuzix_main
	.import kstack_top
	.import kstackc_top
	.import vector
	.import nmi_handler
	.import interrupt_handler
	.import emulation
	.import illegal_inst
	.import trap_inst
	.import abort_inst

	.import  __BSS_RUN__, __BSS_SIZE__
	.importzp	ptr1, ptr2, tmp1

	; startup code @0300
        .include "kernel.def"
        .include "../kernel816.def"
	.include "zeropage.inc"

;
;	So we end up first in the image
;
	.segment "START"
	.byte 65
	.byte 81

	.p816
	.a8
	.i8

entry:
;
;	We are entered at $0302 just after the required magic number
;
;	The big image load is much more complicated as we need to shuffle
;	chunks of it into banks, clear spaces and fire things up
;
;
	sep	#$30		; ensure we are in 8bit mode
	lda	#'F'
	sta	f:$00FE20	; signal our arrival

	sei			; interrupts off
	cld			; decimal off

	rep	#$30
	.a16
	.i16

;
;	Move the stubs up high so the vectors are set and the trampoline
;
	ldx	#$0000
	ldy	#$FF00
	lda	#$00FF
	mvn	#$0,#$0

;
;	Move the stubs into bank 1 
;
	ldx	#$0000
	ldy	#$FF00
	lda	#$00FF
	mvn	#$0,#$1

;
;	Move the code into bank 1
;
	ldx	#$0300
	ldy	#$0300
	lda	#$EEFF
	mvn	#$0,#$1

;
;	Move the data into bank 2
;
	ldx	#$F200
	ldy	#$0300
	lda	#$09FF
	mvn	#$0,#$2

;
;	Wipe the rest (bank was left as 2 here)
;
	ldx	#$0C00
	ldy	#$0C01
	stz	$0C00
	lda	#$EFFE		; Wipe 0C00-FBFF
	mvn	#$2,#$2

;
;	Wipe the low spaces
;
	ldx	#$0C00
	ldy	#$0000
	lda	#$1FF
	mvn	#$2,#$2		; Wipe low memory using clear space in bank 2

	ldx	#$0C00
	ldy	#$0000
	lda	#$1FF
	mvn	#$2,#$0		; Wipe low memory using clear space in bank 2
	

	sep	#$30
	.a8
	.i8


	lda	#'u'
	sta	f:$00FE20

	; jml 1:switch (the assembler is too smart for its own good so
	; do it by hand)
	.byte	$5C
	.word	switch
	.byte	$01

;
;	We are now running in the right bank of code
;

switch:
	lda	#$02
	pha
	plb

;
;	And the right bank of data
;
	rep	#$10
	.i16

	ldx	#kstack_top-1
	txs			; Stack (6502 not C)

	lda	#'z'
	sta	f:$00FE20

	ldx	#kstackc_top-1	; C stack
	stx	sp

	lda	#'i'
	sta	f:$00FE20

	sep #$30
	.a8
	.i8

	lda	#'x'
	sta	f:$00FE20

	jsr	init_early
	lda	#'.'
	sta	f:$00FE20
	jsr	init_hardware
	lda	#13
	sta	f:$00FE20
	lda	#10
	sta	f:$00FE20

	jmp	code

	.code

	.a8
	.i8
code:
	jsr	_fuzix_main	; Should never return
	sei			; Spin
stop:	bra stop

	.segment "STUBS"
;
;	So this ends up in bank 0
;
;	These actually are jump longs to the routine but in bank 1
;
illegal_inst_l:
	.byte $5C
	.word illegal_inst
	.byte $01
trap_inst_l:
	.byte $5C
	.word trap_inst
	.byte $01
abort_inst_l:
	.byte $5C
	.word abort_inst
	.byte $01
nmi_handler_l:
	.byte $5C
	.word nmi_handler
	.byte $01
interrupt_handler_l:
	.byte $5C
	.word interrupt_handler
	.byte $01
emulation_l:
	.byte $5C
	.word emulation
	.byte $01

;
;	Processor vector table (0:0xFFE0)
;
	.segment "VECTORS"


	.word	0		; unused
	.word	0		; unused
	.word	illegal_inst_l	; COP
	.word	trap_inst_l	; BRK
	.word	abort_inst_l	; ABORT
	.word	nmi_handler_l	; NMI
	.word	0		; Unused (native reset)
	.word	interrupt_handler_l

	;
	;	Emulation mode vectors. If called badness occurred
	;
	.word	emulation_l
	.word	emulation_l
	.word	emulation_l
	.word	emulation_l
	.word	emulation_l
	.word	emulation_l
	.word	emulation_l
	.word	emulation_l
