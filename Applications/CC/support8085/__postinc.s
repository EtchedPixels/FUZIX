;
;		TOS = lval of object HL = amount
;
		.export __postinc
		.setcpu 8080
		.code
__postinc:
		xchg
		pop	h
		xthl
		mov	a,m
		sta	__tmp
		add	e
		mov	m,a
		inx	h
		mov	a,m
		sta	__tmp+1
		adc	d
		mov	m,a
                lhld	__tmp
		ret
