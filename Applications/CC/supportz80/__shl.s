		.export __shl
		.export __shlde
		.code

__shl:
		ex	de,hl
		pop	hl
		ex	(sp),hl
__shlde:	ld	a,e
		and	15
		ret	z
shloop:		add	hl,hl
		dec	a
		jr	nz,shloop
		ret
