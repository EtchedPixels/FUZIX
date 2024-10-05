;
;	Switch. We use the tables as is, nothing clever like
;	binary searching yet
;
			.export __switchc
			.export __switchcu
			.setcpu 8080
			.code

__switchcu:
__switchc:
		push	b
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
		mov	a,c
		cmp	m
		inx	h		; Move on to address
		jz	match
		inx	h		; Skip address low
		dcx	d
		mov	a,d
		ora	e
		jnz	next
		inx 	h
		; We are pointing at the address
match:
		mov	e,m
		inx	h
		mov	d,m
		xchg
		pop	b
		pchl
