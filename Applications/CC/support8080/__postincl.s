;
;		TOS = lval of object HL = amount. Amount is never long
;		on an 8085. The compiler generates this knowing that
;		int is safe (largest possible sizeof()).
;
		.export __postincl
		.setcpu 8080
		.code
__postincl:
		xchg
		pop	h
		xthl
		; HL is now the pointer, hireg:DE the amount
		mov	a,m
		sta	__tmp
		add	e
		mov	m,a
		inx	h
		mov	a,m
		sta	__tmp+1
		adc	d
		mov	m,a
		inx	h
		mov	a,m
		sta	__hireg
		aci	0
		mov	m,a
		inx	h
		mov	a,m
		sta	__hireg+1
		aci	0
		mov	m,a
                lhld	__tmp
		ret
