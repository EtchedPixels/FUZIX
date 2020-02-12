
	.text
	.even
	.globl _htons

_htons:
	mov 2(sp),r0
	swab r0
	rts pc
