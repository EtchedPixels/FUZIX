;
;	Sign extend L into HL
;
		.export __sex
		.code

__sex:
		ld	a,l
		ld	h,0
		or 	a
		ret	p
		dec	h
		ret

