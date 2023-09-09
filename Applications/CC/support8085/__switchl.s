;
;	Switch. We use the tables as is, nothing clever like
;	binary searching yet
;
			.export __switchl
			.export __switchlu
			.setcpu 8080
			.code

__switchl:
__switchlu:
		push	b
		mov	b,h
		mov	c,l

		; DE points to the table in the format
		; Length
		; value.32, label.16
		; default label
		xchg
		mov	e,m
		inx	h
		mov	d,m
		; DE is the number of records to scan

next:
		; Low byte compare to C
		inx	h		; Move on to value 0
		mov	a,m
		cmp	c
		jnz	nomatch		; Value 0 didn't match

		; Second byte compare to B
		inx	h		; Move on to value 1
		mov	a,m
		cmp	b		; Value 1 didn't match
		jnz	nomatch1

		; Third byte compare to __hireg
		inx	h
		lda	__hireg
		cmp	m
		jnz	nomatch2

		; Final byte compare to __hireg+1
		inx	h
		lda	__hireg+1
		cmp	m
		jnz	nomatch3
match:
		inx	h		; Point to the address
		mov	e,m
		inx	h
		mov	d,m
		xchg
		pop	b
		pchl



nomatch:
		inx	h
nomatch1:
		inx	h		; Skip value 1
nomatch2:
		inx	h		; Skip value 2
nomatch3:
		inx	h		; Skip value 3
		inx	h		; Skip to end of address
		dcx	d
		mov	a,e
		ora	d
		jnz	next
		jmp	match
