;
;		H holds the pointer
;
		.export __derefl
		.export __dereflsp
		.setcpu	8080
		.code
__dereflsp:
		dad	sp
__derefl:
		mov	e,m
		inx	h
		mov	d,m
		inx	h
		push	d
		mov	e,m
		inx	h
		mov	d,m
		xchg
		shld	__hireg
		pop	h
		ret
