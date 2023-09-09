;
;	int to unsigned long
;
		.export __cast_ul
		.setcpu 8080
		.code

__cast_ul:
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
