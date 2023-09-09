		.export __shrdeu
		.export __shrde
		.export __shru
		.code

__shru:
		ex	de,hl		; shift value into d
		pop	hl		; return address into h
		ex	(sp),hl		; return up stack, h is now the value to shift
		jp	__shrdeu

__shrde:
		ld	a,h
		add	a,a
		jr	c,__shrdeneg
		; Positive right shift signed and unsigned are the same
__shrdeu:
		ld	a,e
		and	15
		ret	z		; no work to do
		cp	8
		jr	c,shrlp
		ld	l,h
		ld	h,0
		sub	8
shrlp:
		srl	h
		rr	l
		dec	a
shftr:		jr	nz,shrlp
		ret

__shrdeneg:
		ld	a,e
		and	15
		ret	z
		cp	8
		jr	c,shrnlp
		ld	l,h
		ld	h,255
shrnlp:
		sra	h
		rr	l
		dec	a
		jr	nz,shrnlp
		ret
