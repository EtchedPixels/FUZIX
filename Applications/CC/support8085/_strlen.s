;
;	strlen
;
		.export _strlen
		.setcpu 8080
		.code

_strlen:
	pop	h
	pop	d
	push	d
	push	h
	lxi	h,0
loop:
	ldax	d
	inx	d
	ora	a
	rz
	inx	h
	jmp	loop
