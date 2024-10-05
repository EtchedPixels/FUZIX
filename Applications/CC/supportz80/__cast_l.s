;
;	int to long
;
		.export __cast_l
		.code

__cast_l:
		ld	a,h
		ld	de,0		; upper word guess 0
		or	a
		jp	p, is_p		; positive
		dec	de		; extend with FFFF
is_p:
		ld	(__hireg),de
		ret
