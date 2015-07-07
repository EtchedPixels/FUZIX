	.section .text.longjmp
	.global longjmp
	.type longjmp, @function
	.func longjmp
longjmp:
	mov	r12, r14 ; param 0: address of jmp_buf
	mov	r13, r12 ; param 1; return value
	mov	@r14+, r1
	mov	@r14+, r2
	mov	@r14+, r4
	mov	@r14+, r5
	mov	@r14+, r6
	mov	@r14+, r7
	mov	@r14+, r8
	mov	@r14+, r9
	mov	@r14+, r10
	mov	@r14+, r11
	br @r14
.endfunc
