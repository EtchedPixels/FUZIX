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
	ldei r0,@rr14
	ldei r1,@rr14	; Return address
	ldei r4,@rr14
	ldei r5,@rr14
	ldei r6,@rr14
	ldei r7,@rr14
	ldei r8,@rr14
	ldei r9,@rr14
	ldei r10,@rr14
	ldei r11,@rr14
	ldei r12,@rr14
	ld 254,r12		; Stack back
	lde r12,@rr14
	ld 255,r12
	push r1
	push r0
	ret

