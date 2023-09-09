;
;		hireg:HL += TOS
;
		.export __plusl
		.setcpu 8080
		.code

__plusl:
	xchg
	pop	h
	shld	__retaddr
	pop	h
	dad	d		; HL is now low part
	pop	d		; High part
	push	h		; Save low part
	lhld	__hireg		; Do high part
	jnc	nocarry		; We need to carry from the first dad
	inx	h		; Carry
nocarry:
	dad	d
	shld	__hireg		; Save high part
	pop	h		; Recover result
	jmp	__ret		; Out
