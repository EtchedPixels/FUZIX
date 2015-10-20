
	.section .text.setjmp

	.global	setjmp
	.type setjmp, @function
	.func setjmp
setjmp:
	pop	r14
	mov	r14, 10*2(r12) ; return address
	mov r1, 0*2(r12)
	mov r2, 1*2(r12)
	mov r4, 2*2(r12)
	mov r5, 3*2(r12)
	mov r6, 4*2(r12)
	mov r7, 5*2(r12)
	mov r8, 6*2(r12)
	mov r9, 7*2(r12)
	mov r10, 8*2(r12)
	mov r11, 9*2(r12)
	clr r12
	br r14
.endfunc

