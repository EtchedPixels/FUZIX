
	.export __ldbyte3

	.setcpu 8080
	.code
__ldbyte3:
	lxi h,3
dad sp
	mov l,m
	ret
