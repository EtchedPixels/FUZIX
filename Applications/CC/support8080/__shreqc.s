;
;	Right shift TOS left by HL
;
			.export __shreqc
			.setcpu 8080
			.code
__shreqc:
	xchg
	pop	h
	xthl
	mov	a,e
	ani	7
	mov	e,a
	mov	a,m
	jz	noop
	ora	a
loop:
	ora	a
	jp	pve
	stc
pve:
	rar
	dcr	e
	jnz	loop
	mov	m,a
noop:
	mov	l,a
	ret


