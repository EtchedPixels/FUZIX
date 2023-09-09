;
;	Sign extend L into HL
;
			.export __sex
			.setcpu 8080
			.code

__sex:
	mov	a,l
	mvi	h,0
	ora 	a
	rp
	dcr	h
	ret
