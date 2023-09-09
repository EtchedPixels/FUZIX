;
;	Cast from char to int or uint
;
		.export __castc_l
		.export __castc_ul
		.setcpu 8080

		.code

__castc_l:
__castc_ul:
	mvi	h,0
	mov	a,l
	ora	a
	jp	positive
	dcr	h
positive:
	mov	a,h
	sta	__hireg
	sta	__hireg+1
	ret
