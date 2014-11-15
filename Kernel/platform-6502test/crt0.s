        	; exported symbols
	        .export start

		; imported symbols
		.import init_early
		.import init_hardware
		.import _fuzix_main

		.import  __BSS_RUN__, __BSS_SIZE__


	        ; startup code @0
	        .code
		.include "zeropage.inc"

start:		
		sei			; interrupts off
		cld			; decimal off
		ldx #$FF
		txs			; Stack (6502 not C)
		
		lda #<kstack_top	; C stack
		sta sp
		lda #>kstack_top
		sta sp+1 

		ld a,#<__BSS_RUN__
		sta ptr1
		ld a,#>__BSS_RUN__
		sta ptr1+1
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

		jsr init_early
		jsr init_hardware
		jsr _fuzix_main		; Should never return
		sei			; Spin
stop:		bra stop
