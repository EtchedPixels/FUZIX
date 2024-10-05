
	.export _devide_read_data
	.export _devide_write_data

	.common

#define IDE_REG_DATA	0x0090

_devide_read_data:
	pop de
	pop hl
	push hl
	push de
	push bc
	ld a, (_td_raw)
	or a
	jr z, read_k
	di
	ld bc,(sio_reg)
	out (c),b
	ld a,(bits_to_user + 1)
	out (c),a
read_k:
	ld bc, IDE_REG_DATA	; port and count
	inir                    ; transfer first 256 bytes
	inir                    ; transfer second 256 bytes
	ld bc,(sio_reg)
	out (c),b
	ld a,(bits_to_user)
	out (c),a
	pop bc
	ld a,(_int_disabled)
	or a
	ret z
	ei
	ret

_devide_write_data:
	pop de
	pop hl
	push hl
	push de
	push bc
	ld a, (_td_raw)
	or a
	jr z, write_k
	di
	ld bc,(sio_reg)
	out (c),b
	ld a,(bits_to_user + 1)
	out (c),a
write_k:
	ld bc, IDE_REG_DATA	; port and count
	otir                    ; transfer first 256 bytes
	otir                    ; transfer second 256 bytes
	ld bc,(sio_reg)
	out (c),b
	ld a,(bits_to_user)
	out (c),a
	pop bc
	ld a,(_int_disabled)
	or a
	ret z
	ei
	ret
