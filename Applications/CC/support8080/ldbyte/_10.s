
	.export __ldbyte10

	.setcpu 8080
	.code
__ldbyte10:
	lxi h,10
dad sp
	mov l,m
	ret
