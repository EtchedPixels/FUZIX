		.65c816
		.a16
		.i16
		.code

.export __text

__text:
		.word 0x80A8		; Fuzix executable
		.byte 2			; 6502 series
		.byte 0x90		; 65C816 16bit
		.byte 1			; Base 0x100
		.byte 0			; No hints

		.word __data-0x100	; code
		.word __data_size	; data
		.word __bss_size	; bss size

		.byte 18		; Run 18 bytes in
		.byte 0			; No size hint
		.byte 0			; No stack hint
		.byte 0			; No ZP on 8080

		.word __sighandler		; signal handler vector

start2:
		tya
		clc
		adc #4
		sta _environ
		lda 2,y
		sta ___argv
		jsr _main		; go
		sta 0,y			; stack cleanup not needed
		jsr _exit		; as never returns

;
;	Signal handling glue. Called to indirect signals from the kernel
;	through code that saves the non-rentrant OS glue. Our stack
;	on entry has the return frame and we are passed C=signal number
;
__sighandler:
		; X is vector Y is C stack A is signal
		; Put the signal number on the C stack
		; the handler will remove it and clean up
		dey
		dey
		sta 0,y
		lda @tmp
		pha
		; TODO others
		jsr jmpx
sigret:		pla	
		sta @tmp
		rts
jmpx:
		phx
		rts

					; kernel helpers
.export _environ
_environ:	.word 0

