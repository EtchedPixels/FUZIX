		.export __shrdec
		.export __shrdeuc
		.setcpu	8080

		.code

__shrdec:
		mov	a,l
		ora	a
		rz			; we can check for free...
		jp	__shrdeuc	; same as unsigned if positive
;
;	Shifting a negative number
;
		mov	a,e
		ani	7
		rz
		mov	e,a
loopm:
		stc
		rar
		dcr	e
		jnz	loopm
		ret
	

__shrdeuc:
		; Shift L right unsigned by e
		mov	a,e
		ani	7
		rz
		mov	e,a
loop:
		ora	a
		rar
		dcr	e
		jnz	loop
		ret
