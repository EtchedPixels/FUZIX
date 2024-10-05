		.export __cmpeqb

		.code

;
;	Tighter version with the other value in DE
;
__cmpeqb:
		ld	a,l
		cp	e
		jp	nz,__false
		jp	__true
