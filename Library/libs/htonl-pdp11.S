
	.text
	.even
	.globl _htonl

_htonl:
	mov 2(sp),r1
	mov 4(sp),r0
	swab r0
	swab r1
	rts pc
