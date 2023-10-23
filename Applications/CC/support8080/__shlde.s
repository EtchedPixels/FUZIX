			.export __shl
			.export __shlde
			.export __shldec
			.setcpu 8080
			.code

; Shift HL left by E

__shl:
		xchg
		pop	h
		xthl
__shlde:
	mov	a,e
__shldec:
	ani	15
	rz
	cpi	8
	jc	noquick
	mov	h,l
	mvi	l,0
	sbi	8
	rz
noquick:
	dad	h
	dcr	a
	jnz	noquick
	ret
