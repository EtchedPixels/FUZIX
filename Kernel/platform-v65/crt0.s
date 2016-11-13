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
		.byte 02

entry:
;
;	We are entered at $2002 just after the required magic number
;
		lda #'F'
		sta $FE20		; signal our arrival

		sei			; interrupts off
		cld			; decimal off
		ldx #$FF
		txs			; Stack (6502 not C)

		lda #'u'
		sta $FE20

		lda #<kstack_top	; C stack
		sta sp
		lda #>kstack_top
		sta sp+1 

		lda #<__BSS_RUN__
		sta ptr1
		lda #>__BSS_RUN__
		sta ptr1+1

		lda #'z'
		sta $FE20

		lda #0
		tay
		ldx #>__BSS_SIZE__
		beq bss_wipe_tail
bss_wiper_1:	sta (ptr1),y
		iny
		bne bss_wiper_1
		inc ptr1+1
		dex
		bne bss_wiper_1

bss_wipe_tail:
		cpy #<__BSS_SIZE__
		beq gogogo
		sta (ptr1),y
		iny
		bne bss_wipe_tail

gogogo:
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
		jsr _fuzix_main		; Should never return
		sei			; Spin
stop:		jmp stop

		.segment "VECTORS"
		.addr	vector
		.addr	$2002		; does it matter ???
		.addr	nmi_handler
