
			.export __castuc_l
			.export __castuc_ul
			.export __castu_l
			.export __castu_ul
			.setcpu 8080
			.code
__castuc_l:
__castuc_ul:
	mvi	h,0
__castu_l:
__castu_ul:
	xchg
	lxi	h,0
	shld	__hireg
	xchg
	ret
