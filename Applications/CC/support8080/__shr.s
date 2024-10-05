;
;	Shift the top of stack right by HL (arithemtic)
;
			.export __shr
			.setcpu 8080
			.code
__shr:
		xchg
		pop	h
		xthl
		mov	a,e
		ani	15
		rz
		mov	a,h
		ora	a
		jp	sr0
shift1:
		mov	a,h
		stc
		rar
		mov	h,a
		mov	a,l
		rar
		mov	l,a
		dcr	e
		jnz	shift1
		ret
sr0:
		mov	a,h
		ora	a
		rar
		mov	h,a
		mov	a,l
		rar
		mov	l,a
		dcr	e
		jnz	sr0
		ret
