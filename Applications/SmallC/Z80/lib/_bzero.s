		.code
		.export _bzero

_bzero:	pop af
	pop hl
	pop de
	push de
	push hl
	push af
	push bc
	ld c,e
	ld b,d
	ld a,b
	or c
	jr z, _bzero_1
	ld (hl),0
	dec bc
	ld a,b
	or c
	jr z,_bzero_1
	ld e,l
	ld d,h
	inc de
	ldir
_bzero_1:
	pop bc
	ret