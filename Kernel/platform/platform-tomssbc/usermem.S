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
	ld ix, 0
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
	ld hl, 0
	ret

__uput:
	push ix
	ld ix, 0
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
	ld hl,#2
	add hl,sp
	push bc
	ld e,(hl)
	inc hl
	ld d,(hl)
	inc hl
	ld c,(hl)
	inc hl
	ld b, (hl)
	ld a,b
	or c
	jr z, done
	ld l,e
	ld h,d
	ld a, 0x01
	out (0x3E),a		; user bank
	ld (hl), 0
	dec bc
	ld a,b
	or c
	jr z, uout
	inc de
	ldir
uout:
	xor a
	out (0x3E),a
done:
	pop bc
	ret

__ugetc:
	pop de
	pop hl
	push hl
	push de
	ld a, 0x01
	out (0x3E),a
	ld l,(hl)
	ld h, 0
ugout:
	xor a
	out (0x3E),a
	ret

__ugetw:
	pop de
	pop hl
	push hl
	push de
	ld a, 0x1
	out (0x3E),a
	ld a,(hl)
	inc hl
	ld h,(hl)
	ld l,a
	jr ugout
	
__uputc:
	ld hl, 2
	add hl,sp
	ld a,(hl)
	inc hl
	inc hl
	ld e,(hl)
	inc hl
	ld d,(hl)
	ld l,a
	ld a, 0x01
	out (0x3E),a
	ex de,hl
	ld (hl), e
	ld hl,0
	jr ugout

__uputw:
	ld hl, 2
	add hl,sp
	ld e,(hl)
	inc hl
	ld d,(hl)
	inc hl
	ld a,(hl)
	inc hl
	ld h,(hl)
	ld l,a
	ld a, 0x01
	out (0x3E),a
	ld (hl),e
	inc hl
	ld (hl),d
	ld hl,0
	jr ugout
