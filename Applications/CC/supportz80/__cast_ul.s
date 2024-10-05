;
;	int to unsigned long
;
		.export __cast_ul
		.code

__cast_ul:
		ld	a,h
		ld	de,0		; upper word guess 0
		or	a
		jp	is_p		; positive
		dec	de		; extend with FFFF
is_p:
		ld	(__hireg),de
		ret
