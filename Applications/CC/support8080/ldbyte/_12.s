
	.export __ldbyte12

	.setcpu 8080
	.code
__ldbyte12:
	lxi h,12
dad sp
	mov l,m
	ret
