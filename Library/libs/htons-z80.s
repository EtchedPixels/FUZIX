;
;	SDCC makes a bit of a mess of this so do it by hand
;
		.area _CODE

		.globl _htons

_htons:
	pop de
	pop hl
	push hl
	push de
	ld a,h
	ld h,l
	ld l,a
	ret
