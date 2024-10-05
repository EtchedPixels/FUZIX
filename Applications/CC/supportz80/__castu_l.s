
		.export __castuc_l
		.export __castuc_ul
		.export __castu_l
		.export __castu_ul

		.code
__castuc_l:
__castuc_ul:
		ld	h,0
__castu_l:
__castu_ul:
		ld	de,0
		ld	(__hireg),de
		ret

