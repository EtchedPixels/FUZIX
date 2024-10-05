;
;	Right shift TOS left by HL
;
			.export __shrequc
			.setcpu 8080
			.code
__shrequc:
	xchg
	pop	h
	xthl
	mov	a,e
	ani	7
	mov	e,a
	mov	a,m
	jz	noop
loop:
	ora	a
	rar
	dcr	e
	jnz	loop
	mov	m,a
noop:
	mov	l,a
	ret


