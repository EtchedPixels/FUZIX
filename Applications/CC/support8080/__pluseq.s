;
;		TOS = lval of object HL = amount
;
		.export __pluseq

		.setcpu 8080
		.code
__pluseq:
		xchg			; amount into D
		pop	h		; return
		xthl			; swap with lval
		xchg			; get lval into D
		push	d		; save lval
		push	h		; save value to add
		xchg
		mov	e,m
		inx	h
		mov	d,m
		pop	h		; get value back
		dad	d		; add __tmp to it
		xchg
		pop	h		; get the TOS address
		mov	m,e
		inx	h
		mov	m,d
		xchg
		ret
