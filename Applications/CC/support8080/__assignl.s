;
;	Assign the value in hireg:HL to lval at tos.
;
		.export __assignl
		.export	__assign0l
		.setcpu 8080
		.code

__assignl:
		xchg			; hireg:de is our value
		pop	h
		xthl			; hl is now our pointer
		mov	m,e
		inx	h
		mov	m,d
		inx	h
		push	d		; for return
		xchg
		lhld	__hireg
		xchg
		; de is now the high bytes
		mov	m,e
		inx	h
		mov	m,d
		pop	h		; saved low word
		ret


; Assign 0L to lval in HL
__assign0l:
		xra	a
		mov	m,a
		inx	h
		mov	m,a
		inx	h
		mov	m,a
		inx	h
		mov	m,a
		mov	h,a
		mov	l,a
		shld	__hireg		; clear hireg
		ret
