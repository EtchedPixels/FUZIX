		.export __shrdeu
		.export __shrde
		.export __shru
		.setcpu 8080
		.code

__shru:
		xchg		; shift value into d
		pop	h	; return address into h
		xthl		; return up stack, h is now the value to shift
		jmp	__shrdeu

;
;	For the 8080 we have to go via A all the time
;
__shrde:
		mov	a,h
		ora	a
		jm	__shrdeneg
		; Positive right shift signed and unsigned are the same
__shrdeu:
		mov	a,e
		ani	15
		rz		; no work to do
		mov	e,a
shrlp:
		mov	a,h
		ora	a
		rar
		mov	h,a
		mov	a,l
		rar
		mov	l,a
		dcr	e
shftr:		jnz	shrlp
		ret

__shrdeneg:
		mov	a,e
		ani	15
		rz
		mov	e,a
shrnlp:
		mov	a,h
		stc
		rar
		mov	h,a
		mov	a,l
		rar
		mov	l,a
		dcr	e
		jnz	shrnlp
		ret
