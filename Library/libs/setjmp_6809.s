; setjmp / longjmp for 6809 FUZIX
; Copyright 2015 Tormod Volden

	; exported
	.globl _setjmp
	.globl _longjmp

	.area .text

; int setjmp(jmp_buf)
_setjmp:
	ldd ,s		; return address
	sty ,x++
	stu ,x++
	sts ,x++
	std ,x
	ldx #0
	rts

; void longjmp(jmp_buf, int)
_longjmp:
	; read back Y,U,S and return address
	ldd 2,s		; second argument
	bne nz		; must not be 0
	incb
nz	ldy ,x++
	ldu ,x++
	lds ,x++	; points to clobbered return address
	ldx ,x
	stx ,s		; restore return address
	; return given argument to setjmp caller
	tfr d,x
	rts

