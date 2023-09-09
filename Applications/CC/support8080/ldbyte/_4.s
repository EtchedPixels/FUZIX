
	.export __ldbyte4

	.setcpu 8080
	.code
__ldbyte4:
	lxi h,4
dad sp
	mov l,m
	ret
