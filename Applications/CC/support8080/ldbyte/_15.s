
	.export __ldbyte15

	.setcpu 8080
	.code
__ldbyte15:
	lxi h,15
dad sp
	mov l,m
	ret
