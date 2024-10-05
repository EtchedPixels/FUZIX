		.export __shrdec
		.export __shrdeuc

		.code

__shrdec:
		ld	a,e
		and	7
		ret	z
		ld	e,a
loopm:
		sra	l
		dec	e
		jr	nz,loopm
		ret
	

__shrdeuc:
		; Shift L right unsigned by e
		ld	a,e
		and	7
		ret	z
		ld	e,a
loop:
		srl	l
		dec	e
		jr	nz,loop
		ret
