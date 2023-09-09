;
;	Cast from char to int or uint
;
		.export __castc_l
		.export __castc_ul

		.code

__castc_l:
__castc_ul:
		ld	h,0
		ld	a,l
		or	a
		jp	p,positive
		dec	h
positive:
		ld	a,h
		ld	(__hireg),a
		ld	(_hireg+1),a
		ret

