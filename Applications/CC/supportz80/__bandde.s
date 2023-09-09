		.export __bandde
		.export __bbandde
		.code

__bandde:
		ld a,d
		and h
		ld h,a
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
