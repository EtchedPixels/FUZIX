;
;	Saved registers are r4-r11
;	We also need to save sp and the return address
;

	.export __setjmp

	.code
__setjmp:
	pop r15		; return
	pop r14	
	pop r13		; buffer
	pop r12
	lde @rr12,r14
	incw rr12
	lde @rr12,r15
	incw rr12
	lde @rr12,r4
	incw rr12
	lde @rr12,r5
	incw rr12
	lde @rr12,r6
	incw rr12
	lde @rr12,r7
	incw rr12
	lde @rr12,r8
	incw rr12
	lde @rr12,r9
	incw rr12
	lde @rr12,r10
	incw rr12
	lde @rr12,r11
	incw rr12
	ld r0,254
	lde @rr12,r0
	incw rr12
	ld r0,255
	lde @rr12,r0
	clr r2
	clr r3
	ret

