;
;	We have a custom implementation of usermem. We really need
;	to optimize ldir_to/from_user.
;
#include "kernelu.def"
#include "../../cpu-z80u/kernel-z80.def"

        ; exported symbols
        .export __uget
        .export __ugetc
        .export __ugetw

        .export __uput
        .export __uputc
        .export __uputw
        .export __uzero

	.common

uputget:
        ; load DE with the byte count
        ld c, (ix + 8) ; byte count
        ld b, (ix + 9)
	ld a, b
	or c
	ret z		; no work
        ; load HL with the source address
        ld l, (ix + 4) ; src address
        ld h, (ix + 5)
        ; load DE with destination address
        ld e, (ix + 6)
        ld d, (ix + 7)
	ret

__uget:
	push ix
	ld ix,0
	add ix, sp
	push bc
	call uputget
	jr z, uget_out
	push de
	pop ix
	call ldir_from_user
uget_out:
	pop bc
	pop ix
	ld hl,0
	ret

__uput:
	push ix
	ld ix,0
	add ix,sp
	push bc
	call uputget
	jr z, uget_out
	push de
	pop ix
	call ldir_to_user
	jr uget_out

;
;	The kernel IRQ code will restore the bank if it interrupts this
;	logic
;
__uzero:
	ld hl,5
	add hl,sp
	push bc
	ld b,(hl)
	dec hl
	ld c,(hl)
	dec hl
	ld a,(hl)
	dec hl
	ld l,(hl)
	ld h,a
	ld a,b
	or c
	jr z,uzout
	ld a,0x81
	out (0x38),a		; user bank
	rlca
	out (0x30),a
	ld (hl),0
	dec bc
	ld a,b
	or c
	jr z, uzout
	ld e,l
	ld d,h
	inc de
	ldir
uzout:
	ld a,1
	out (0x38),a
	rlca
	out (0x30),a
	pop bc
	ret

__ugetc:
	pop de
	pop hl
	push hl
	push de
	ld a,0x81
	out (0x38),a
	rlca
	out (0x30),a
	ld l,(hl)
	ld h,#0
	jp map_kernel

__ugetw:
	pop de
	pop hl
	push hl
	push de
	ld a,0x81
	out (0x38),a
	rlca
	out (0x30),a
	ld a,(hl)
	inc hl
	ld h,(hl)
	ld l,a
	jp map_kernel
	
__uputc:
	ld hl,2
	add hl,sp
	ld e,(hl)
	inc hl
	ld d,(hl)
	inc hl
	ld a,(hl)
	inc hl
	ld h,(hl)
	ld l,a
	ld a, 0x81
	out (0x38),a
	rlca
	out (0x30),a
	ld (hl),e
	ld hl,0
	jp map_kernel

__uputw:
	ld hl,2
	add hl,sp
	ld e,(hl)
	inc hl
	ld d,(hl)
	inc hl
	ld a,(hl)
	inc hl
	ld h,(hl)
	ld l,a
	ld a, 0x81
	out (0x38),a
	rlca
	out (0x30),a
	ld (hl),e
	inc hl
	ld (hl),d
	ld hl,0
	jp map_kernel
