
	.export __ldbyte2

	.setcpu 8080
	.code
__ldbyte2:
	lxi h,2
dad sp
	mov l,m
	ret
