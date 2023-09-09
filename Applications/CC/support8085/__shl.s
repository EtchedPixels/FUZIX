		.export __shl
		.export __shlde
		.setcpu	8080
		.code

__shl:
		xchg
		pop	h
		xthl
__shlde:	mov	a,e
		ani	15
		rz
shloop:		dad	h
		dcr	a
		jnz	shloop
		ret
