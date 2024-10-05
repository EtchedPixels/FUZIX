;
;	Compare two objects in memory unsigned and set C NC and Z
;	accordingly. Eats the two pointers in H and D
;
			.export __cmpul
			.export __cmpulws
			.export __cmpl
			.export __cmplws
			.setcpu 8080
			.code

__cmpulws:		; workspace and stack
	shld	__tmp	; word before __hireg
	lxi	h,7	; stacked value, high byte
	dad	sp
	lxi	d,__hireg+1	; high byte first
__cmpul:
	ldax	d
	cmp	m
	rnz
	dcx	d
	dcx	h
	ldax	d
	cmp	m
	rnz
	dcx	d
	dcx	h
	ldax	d
	cmp	m
	rnz
	dcx	d
	dcx	h
	ldax	d
	cmp	m
	ret

__cmplws:
	shld	__tmp
	lxi	h,7
	dad	sp
	lxi	d,__hireg+1
__cmpl:
	; Same idea but we need to deal with signs first
	ldax	d
	xra	m
	; Same sign - unsigned compare
	jp	__cmpul
	xra	m
	jm	setnc	; m +ve / d -ve
	inr	a	; clear carry
	; A cannot be 0xFF so this will ensure NZ
	ret
setnc:	xra	a
	inr	a	; NZ
	stc		; NZ and C
	ret
	
