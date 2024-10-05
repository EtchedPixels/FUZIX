;
;	Shift TOS left by HL
;
			.export __shleqc
			.setcpu 8080
			.code
__shleqc:
	xchg
	pop	h
	xthl
	mov	a,e
	ani	7
	mov	e,a
	mov	a,m
	jz	noop
loop:
	add	a
	dcr	e
	jnz	loop
	mov	m,a
noop:
	mov	l,a
	ret


