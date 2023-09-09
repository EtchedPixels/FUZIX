
	.export __ldbyte24

	.setcpu 8080
	.code
__ldbyte24:
	lxi h,24
dad sp
	mov l,m
	ret
