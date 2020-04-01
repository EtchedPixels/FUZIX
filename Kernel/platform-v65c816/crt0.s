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

	; startup code @0200
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
;	We are entered at $0102 just after the required magic number
;
;	We get run from bank 0, our I/O writes would otherwise need to be
;	24bit
;
	sep	#$30		; ensure we are in 8bit mode
	lda	#'F'
	sta	$FE20		; signal our arrival

	sei			; interrupts off
	cld			; decimal off


	; vectors is packed in DP, move it to FF00
	rep	#$30
	.a16
	.i16
	lda	#255
	ldx	#0
	ldy	#$FF00
	mvn	#KERNEL_FAR,#KERNEL_FAR

	sep	#$30
	.a8
	.i8


	lda	#'u'
	sta	$FE20

	rep	#$10
	.i16

	ldx	#kstack_top-1
	txs			; Stack (6502 not C)

	lda	#'z'
	sta	$FE20

	ldx	#kstackc_top-1	; C stack
	stx	sp

	ldx	#__BSS_RUN__

	lda	#'i'
	sta	$FE20

	txy
	iny

	; Wipe the BSS

	rep #$20
	.a16

	lda	#__BSS_SIZE__-2	; must be >=2  bytes or else
	stz	a:0,x
	mvn	#0,#0
		
	sep #$30
	.a8
	.i8

	lda	#'x'
	sta	$FE20

	jsr	init_early
	lda	#'.'
	sta	$FE20
	jsr	init_hardware
	jmp	code

	.code

	.a8
	.i8
code:
	lda	#13
	sta	$FE20
	lda	#10
	sta	$FE20
	jsr	_fuzix_main	; Should never return
	sei			; Spin
stop:	bra stop


;
;	Processor vector table (0xFFE0)
;
	.segment "VECTORS"


	.word	0		; unused
	.word	0		; unused
	.word	illegal_inst	; COP
	.word	trap_inst	; BRK
	.word	abort_inst	; ABORT
	.word	nmi_handler	; NMI
	.word	0		; Unused (native reset)
	.word	interrupt_handler

	;
	;	Emulation mode vectors. If called badness occurred
	;
	.word	emulation
	.word	emulation
	.word	emulation
	.word	emulation
	.word	emulation
	.word	emulation
	.word	emulation
	.word	emulation


