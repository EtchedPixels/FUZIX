	.section .text.longjmp
	.global longjmp
	.type longjmp, @function
	.func longjmp
longjmp:
	mov	r15, r13 ; param 0: address of jmp_buf
	mov	r14, r15 ; param 1; return value
	movx.a @r13+, r1
	movx.a @r13+, r2
	movx.a @r13+, r4
	movx.a @r13+, r5
	movx.a @r13+, r6
	movx.a @r13+, r7
	movx.a @r13+, r8
	movx.a @r13+, r9
	movx.a @r13+, r10
	movx.a @r13+, r11
	br.a @r13
.endfunc
