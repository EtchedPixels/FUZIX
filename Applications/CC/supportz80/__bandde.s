		.export __bandde
		.export __bandde0d
		.export __bbandde
		.code

__bandde0d:
		ld h,0
		jr b1
__bandde:
		ld a,d
		and h
		ld h,a
b1:
		ld a,e
		and l
		ld l,a
		ret
;
;	Little helper for the if (a & b) ... case
;
__bbandde:
		call __bandde
		jp __bool
