		.export __bcmul
		.export __bcmulc

		.setcpu 8080

		.code

		; HL has one value BNC the other.. so just use the helper
__bcmulc:
		mvi	b,0
__bcmul:	
		mov	e,c
		mov	d,b
		call	__mulde		; HL * DE into HL
		mov	c,l
		mov	b,h
		ret
