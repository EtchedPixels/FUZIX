;
;	Cast from char to int or uint
;
		.export __castc_
		.export __castc_u

		.code

__castc_:
__castc_u:
		ld	h,0
		ld	a,l
		or	a
		ret	p
		dec	h
		ret

