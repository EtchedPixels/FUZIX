		; imported symbols
		.import init_early
		.import init_hardware
		.import _fuzix_main
		.import kstack_top
		.import vector
		.import nmi_handler

		.import  __BSS_RUN__, __BSS_SIZE__
		.importzp	ptr1, ptr2, tmp1

	        ; startup code @0
		.include "zeropage.inc"

;
;	So we end up first in the image
;
	        .segment "START"
		.byte 65
		.byte 81

		.a8
		.i8
		.p816

entry:
;
;	We are entered at $0202 just after the required magic number
;
;	We get run from bank 0, our I/O writes would otherwise need to be
;	24bit
;
		sep #$30		; ensure we are in 8bit mode
		lda #'F'
		sta $FE20		; signal our arrival

		sei			; interrupts off
		cld			; decimal off

		rep #$10
		.i16
		ldx #kstack
		txs			; Stack (6502 not C)

		lda #'u'
		sta $FE20

		ldx #kstack_top	; C stack
		sta sp

		ldx #__BSS_RUN__

		lda #'z'
		sta $FE20

		txy
		iny

		; Wipe the BSS

		rep #$20
		.a16
		lda #__BSS_SIZE-2	; must be >=2  bytes or else
		clz 0,x
		mvn 0,0

		
		sep #$30
		.a8
		.i8

		lda #'i'
		sta $FE20

		lda #'x'
		sta $FE20

		jsr init_early
		lda #'.'
		sta $FE20
		jsr init_hardware
		lda #13
		sta $FE20
		lda #10
		sta $FE20
		jmp code

; The above gets blasted into udata space
		.code

code:
		rep #$30
		.a8
		.i8
		ldx #$U_DATA
		ldy #$U_DATA+1
		lda #$UDATA_TOTALSIZE-2
		clz 0,x
		mvn 0,0

		sep #$30
		.a8
		.i8

		jsr _fuzix_main		; Should never return
		sei			; Spin
stop:		jmp stop

		.segment "VECTORS"
		.addr	vector
		.addr	$0202		; does it matter ???
		.addr	nmi_handler
