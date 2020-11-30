	.area _FLASH
	jp unlock_flash
	jp lock_flash

unlock_flash:
	ld a, i
	jp pe, ufc
	ld a, i
ufc:	push af
	di
	ld a, #1
	nop
	nop
	im 1
	di
	out (0x14), a
	im 2
	pop af
	ret po
	ei
	ret

lock_flash:
	ld a, i
	jp pe, lfc
	ld a, i
lfc:	push af
	di
	xor a
	nop
	nop
	im 1
	di
	out (0x14), a
	im 2
	pop af
	ret po
	ei
	ret
