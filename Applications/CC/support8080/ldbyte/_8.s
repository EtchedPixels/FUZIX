
	.export __ldbyte8

	.setcpu 8080
	.code
__ldbyte8:
	lxi h,8
dad sp
	mov l,m
	ret
