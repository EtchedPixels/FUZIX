;
;	TODO: tms9995 support
;
;
	.export __setjmp
	.code

__setjmp:
	mov	*r13, r0
	mov	r13, *r0+		; stack pointer
	mov	r11, *r0+		; return
	mov	r8, *r0+
	mov	r9, *r0+
	mov	r10, *r0+
	mov	r11, *r0+
	mov	r12, *r0+
	mov	r14, *r0+
	mov	r15, *r0+
	mov	@fp1, *r0+
	mov	@fp1+2, *r0+
	mov	@fp2, *r0+
	mov	@fp2+2, *r0+
	clr	r1
	rt

