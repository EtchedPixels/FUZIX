	.65c816
	.a16
	.i16

	.export _longjmp

_longjmp:
	lda 4,y
	bne valok
	inc a
valok:
	sta @tmp
	lda 2,y
	ldx 0,y
	tay		; set stack back
	lda @tmp	; return value
	phx		; and return to the setjmp frame
	rts
