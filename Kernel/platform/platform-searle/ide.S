#include "kernelu.def"

#define IDE_REG_DATA	0x10

/*
 *	We need slightly custom transfer routines for the IDE controller
 *	as we have no common stack to use.
 *
 *	As we only have one bank of user it's a bit simpler
 */

	.common
	.export _devide_read_data
	.export _devide_write_data

_devide_read_data:
	pop de
	pop hl
	push hl
	push de
	push bc
	ld a, (_td_raw)
	ld bc, IDE_REG_DATA                    ; setup port number
	                                        ; and count
	or a
	jr z, via_kernel
	di
	ld a, 0x01
	out (0x03),a
	ld a, 0x58
	out (0x03),a            
via_kernel:
	inir                                    ; transfer first 256 bytes
	inir                                    ; transfer second 256 bytes
	ld a, 0x01
	out (0x03),a
	ld a, 0x18
	out (0x03),a
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
	ld bc, IDE_REG_DATA                    ; setup port number
	                                        ; and count
	or a
	jr z, wvia_kernel
	di
	ld a, 0x01
	out (0x03),a
	ld a, 0x58
	out (0x03),a            
wvia_kernel:            
	otir                                    ; transfer first 256 bytes
	otir                                    ; transfer second 256 bytes
	ld a, 0x01
	out (0x03),a
	ld a, 0x18
	out (0x03),a
	pop bc
	ld a,(_int_disabled)
	or a
	ret z
	ei
	ret
