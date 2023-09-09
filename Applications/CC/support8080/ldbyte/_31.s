
	.export __ldbyte31

	.setcpu 8080
	.code
__ldbyte31:
	lxi h,31
dad sp
	mov l,m
	ret
