;
;	Shift the top of stack right by HL (arithemtic)
;
			.export __shr
			.setcpu 8085
			.code
__shr:
		xchg
		pop	h
		xthl
		mov	a,e
		ani	15
		rz
shift1:
		arhl
		dcr	e
		jnz	shift1
		ret

