
	.export _devide_read_data
	.export _devide_write_data

	.common

#define IDE_REG_DATA	0x0010

_devide_read_data:
	pop de
	pop hl
	push hl
	push de
	push bc
	ld bc, IDE_REG_DATA	; port and count
	ld a, (_td_raw)
	cp #2
	jr nz, not_swapin
	ld a, (_td_page)			    ; swap page
not_swapin:
	; At this point A = 0 kernel, A = 1, user
	out (0x3E),a
	inir                                    ; transfer first 256 bytes
	inir                                    ; transfer second 256 bytes
	xor a
	out (0x3E),a
	pop bc
	ret

_devide_write_data:
	pop de
	pop hl
	push hl
	push de
	push bc
	ld bc, IDE_REG_DATA	; port and count
	ld a, (_td_raw)
	cp #2
	jr nz, not_swapout
	ld a, (_td_page)			    ; swap page
not_swapout:
	; At this point A = 0 kernel, A = 1, kernel
	out (0x3E),a
	otir                                    ; transfer first 256 bytes
	otir                                    ; transfer second 256 bytes
	xor a
	out (0x3E),a
	pop bc
	ret
