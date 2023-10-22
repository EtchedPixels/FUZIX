		.export __shl
		.export __shlde
		.export __shlde0d
		.export __shldec
		.code

; Shift HL left by E
__shl:
		ex	de,hl
		pop	hl
		ex	(sp),hl
__shlde0d:
__shlde:
		ld	a,e
__shldec:
		and	15
		ret	z
		cp	8
		jr	c,noquick
		ld	h,l
		ld	l,0
		sub	8
		ret	z
noquick:
		add	hl,hl
		dec	a
		jr	nz,noquick
		ret

