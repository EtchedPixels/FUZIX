;
;	int to long
;
		.export __cast_l
		.setcpu 8080
		.code

__cast_l:
		mov	a,h
		lxi	d,0		; upper word guess 0
		ora	a
		jp	is_p		; positive
		dcx	d		; extend with FFFF
is_p:
		xchg
		shld	__hireg
		xchg
		ret
