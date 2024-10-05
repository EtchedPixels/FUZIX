		.export __bcmul
		.export __bcmulc

		.code

		; HL has one value BNC the other.. so just use the helper
__bcmulc:
		ld	b,0
__bcmul:	
		ld	e,c
		ld	d,b
		call	__mulde		; HL * DE into HL
		ld	c,l
		ld	b,h
		ret
