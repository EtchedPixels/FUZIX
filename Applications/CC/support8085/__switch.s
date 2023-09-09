;
;	Switch. We use the tables as is, nothing clever like
;	binary searching yet
;
			.export __switch
			.export __switchu
			.setcpu 8080
			.code

__switchu:
__switch:
		push	b
		mov	b,h
		mov	c,l

		; DE points to the table in the format
		; Length
		; value, label
		; default label
		xchg
		mov	e,m
		inx	h
		mov	d,m
next:
		inx	h		; Move on to value to check
		mov	a,m
		cmp	c
		inx	h		; Move on to address
		mov	a,m
		inx	h
		jnz	nomatch
		cmp	b
		jz	match
nomatch:
		inx	h		; Skip address low
		dcx	d
		mov	a,e
		ora	d
		jnz	next
		inx	h
		; We are pointing at the address
match:
		mov	e,m
		inx	h
		mov	d,m
		xchg
		pop	b
		pchl
