;
;		True and False helpers. Set H and flags
;
		.export __true
		.export	__false
		.export __rtrue
		.export	__rfalse

__true:
		ld	hl,0
		inc	l
		ret
__false:
		xor	a
		ld	h,a
		ld	l,a
		ret

__rtrue:
		ld	hl,0
		inc	l
		jp	__ret

__rfalse:	xor	a
		ld	h,a
		ld	l,a
		jp	__ret
