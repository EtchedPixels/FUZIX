
	.export	_longjmp

	.code

_longjmp:
	mov	*r13, r0		; buffer
	mov	@2(r13),r1		; code
	jne	@retgood
	inc	r1
retgood:
	mov	*r0+, r13
	mov	*r0+, r11
	mov	*r0+, r8
	mov	*r0+, r9
	mov	*r0+, r10
	mov	*r0+, r11
	mov	*r0+, r12
	mov	*r0+, r14
	mov	*r0+, r15
	mov	*r0+, @fp1
	mov	*r0+, @fp1+2
	mov	*r0+, @fp2
	mov	*r0+, @fp2+1
	b	*r11
