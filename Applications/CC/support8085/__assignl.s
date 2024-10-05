;
;	Assign the value in hireg:HL to lval at tos.
;
		.export __assignl
		.export	__assign0l
		.setcpu 8085
		.code

__assignl:
		xchg			; hireg:de is our value
		pop	h
		xthl			; hl is now our pointer
		xchg			; pointer into de, value into hl
		shlx			; save low value
		push	h		; for return
		inx	d
		inx	d
		lhld	__hireg
		shlx
		pop	h
		ret


__assign0l:
		xchg			; address into d
		lxi	h,0
		shlx			; clear lval lower
		shld	__hireg		; clear hireg
		inx	d
		inx	d		; clear upper of lval
		shlx
		ret
