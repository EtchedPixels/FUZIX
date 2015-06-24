
	.section .text.setjmp

	.global	setjmp
	.type setjmp, @function
	.func setjmp
setjmp:
	popx r14
	movx r14, 10*4(r15) ; return address
	movx.a r1, 0*4(r15)
	movx.a r2, 1*4(r15)
	movx.a r4, 2*4(r15)
	movx.a r5, 3*4(r15)
	movx.a r6, 4*4(r15)
	movx.a r7, 5*4(r15)
	movx.a r8, 6*4(r15)
	movx.a r9, 7*4(r15)
	movx.a r10, 8*4(r15)
	movx.a r11, 9*4(r15)
	clrx r15
	br.a r14
.endfunc

