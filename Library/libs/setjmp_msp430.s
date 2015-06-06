
	.section .text.setjmp

	.global	setjmp
	.type setjmp, @function
	.func setjmp
setjmp:
	pop	r14
	mov	r14, 10*2(r15) ; return address
	mov r1, 0*2(r15)
	mov r2, 1*2(r15)
	mov r4, 2*2(r15)
	mov r5, 3*2(r15)
	mov r6, 4*2(r15)
	mov r7, 5*2(r15)
	mov r8, 6*2(r15)
	mov r9, 7*2(r15)
	mov r10, 8*2(r15)
	mov r11, 9*2(r15)
	clr r15
	br r14
.endfunc

