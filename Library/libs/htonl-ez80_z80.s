;
;	SDCC makes a bit of a mess of this so do it by hand
;
		.area _CODE

		.globl _htonl

_htonl:
	pop bc
	pop hl
	pop de
	push de
	push hl
	push bc
	ld a,l
	ld l,d
	ld d,a
	ld a,h
	ld h,e
	ld e,a
	ret
