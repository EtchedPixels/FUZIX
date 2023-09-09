		.export __cmpeq
		.code

;
;	Tighter version with the other value in DE
;
__cmpeq:
		or	a
		sbc	hl,de
		jp	nz,__false
		;	was 0 so now 1 and NZ
		inc	l
		ret
