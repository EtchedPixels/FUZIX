;
;
;	When we knock this into proper shape it'll need to begin by using
;	ProDOS to load the chunks of image and stuff them into bank 1, then
;	kill off ProDOS and move them into the language space.
;
;	For now we put discard low, so we can in theory switch video mode
;	and blow it away (at least for lower resolution modes)
;
;
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

entry:
;
;	We are entered at $2000 from ProDOS
;
		lda #'1'
		sta $0400		; signal our arrival

		sei			; interrupts off
		cld			; decimal off
		ldx #$FF
		txs			; Stack (6502 not C)

		sta $C000		; 40 column
		sta $C050		; text
		sta $C054		; page 1
		sta $C00F		; alt char set on
		sta $C000		; 80 store off

		lda #'2'
		sta $0400

		lda #<kstack_top	; C stack
		sta sp
		lda #>kstack_top
		sta sp+1 

		lda #<__BSS_RUN__
		sta ptr1
		lda #>__BSS_RUN__
		sta ptr1+1

		lda #'3'
		sta $0400

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
		lda #'4'
		sta $0400

		jsr init_early
		lda #'5'
		sta $0400
		jsr init_hardware
		lda #'6'
		sta $0400
		jsr _fuzix_main		; Should never return
		sei			; Spin
stop:		jmp stop

		.segment "VECTORS"
		.addr	vector
		.addr	$2000		; does it matter ???
		.addr	nmi_handler
