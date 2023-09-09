;
;		TOS = lval of object HL = amount
;		Amount is always within the size of a pointer so is not
;		turned long on 8085
;
		.export __postdecl

		.setcpu 8080
		.code

__postdecl:
		xchg
		pop	h
		xthl
		mov	a,m
		sta	__tmp
		sub	e
		mov	m,a
		inx	h
		mov	a,m
		sta	__tmp+1
		sbb	d
		mov	m,a
		inx	h
		mov	a,m
		sta	__hireg
		sbi	0
		mov	m,a
		inx	h
		mov	a,m
		sta	__hireg+1
		sbi	0
		mov	m,a
                lhld	__tmp
		ret
