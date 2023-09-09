
	.export __ldbyte5

	.setcpu 8080
	.code
__ldbyte5:
	lxi h,5
dad sp
	mov l,m
	ret
