	.section .text.longjmp
	.global longjmp
	.type longjmp, @function
	.func longjmp
longjmp:
	mov	r15, r13 ; param 0: address of jmp_buf
	mov	r14, r15 ; param 1; return value
	mov	@r13+, r1
	mov	@r13+, r2
	mov	@r13+, r4
	mov	@r13+, r5
	mov	@r13+, r6
	mov	@r13+, r7
	mov	@r13+, r8
	mov	@r13+, r9
	mov	@r13+, r10
	mov	@r13+, r11
	br @r13
.endfunc
