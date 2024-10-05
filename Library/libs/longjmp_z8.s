;
;	Unwind a setjmp frame
;
	.export _longjmp

	.code
_longjmp:
	incw 254	; throw away the return address
	incw 254
	pop r15
	pop r14
	pop r3		; return code
	pop r2
	ld r0,r2
	or r0,r3
	jr nz, sane_ret
	inc r3		; 0 returns 1
sane_ret:
	lde r0,@rr14
	incw rr14
	lde r1,@rr14	; Return address
	incw rr14
	lde r4,@rr14
	incw rr14
	lde r5,@rr14
	incw rr14
	lde r6,@rr14
	incw rr14
	lde r7,@rr14
	incw rr14
	lde r8,@rr14
	incw rr14
	lde r9,@rr14
	incw rr14
	lde r10,@rr14
	incw rr14
	lde r11,@rr14
	incw rr14
	lde r12,@rr14
	ld 254,r12		; Stack back
	incw rr14
	lde r12,@rr14
	ld 255,r12
	push r1
	push r0
	ret

