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
	ldepi @rr12,r15
	ldepi @rr12,r4
	ldepi @rr12,r5
	ldepi @rr12,r6
	ldepi @rr12,r7
	ldepi @rr12,r8
	ldepi @rr12,r9
	ldepi @rr12,r10
	ldepi @rr12,r11
	ld r0,254
	ldepi @rr12,r0
	ld r0,255
	ldepi @rr12,r0
	clr r2
	clr r3
	ret

