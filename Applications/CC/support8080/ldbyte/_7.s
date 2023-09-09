
	.export __ldbyte7

	.setcpu 8080
	.code
__ldbyte7:
	lxi h,7
dad sp
	mov l,m
	ret
