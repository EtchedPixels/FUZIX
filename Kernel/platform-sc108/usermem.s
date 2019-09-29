;
;	We have a custom implementation of usermem. We really need
;	to optimize ldir_to/from_user.
;
        .module usermem

	.include "kernel.def"
        .include "../kernel-z80.def"

        ; exported symbols
        .globl __uget
        .globl __ugetc
        .globl __ugetw

        .globl __uput
        .globl __uputc
        .globl __uputw
        .globl __uzero

	.globl ldir_from_user
	.globl ldir_to_user

        .area _COMMONMEM


uputget:
        ; load DE with the byte count
        ld c, 8(ix) ; byte count
        ld b, 9(ix)
	ld a, b
	or c
	ret z		; no work
        ; load HL with the source address
        ld l, 4(ix) ; src address
        ld h, 5(ix)
        ; load DE with destination address
        ld e, 6(ix)
        ld d, 7(ix)
	ret

__uget:
	push ix
	ld ix,#0
	add ix, sp
	call uputget
	jr z, uget_out
	push de
	pop ix
	call ldir_from_user
uget_out:
	pop ix
	ld hl,#0
	ret

__uput:
	push ix
	ld ix,#0
	add ix,sp
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
	pop de
	pop hl
	pop bc
	push bc
	push hl
	push de
	ld a,b
	or c
	ret z
	ld a,#0x81
	out (0x38),a		; user bank
	rlca
	out (0x30),a
	ld (hl),#0
	dec bc
	ld a,b
	or c
	jr z, uout
	ld e,l
	ld d,h
	inc de
	ldir
uout:
	ld a,#1
	out (0x38),a
	rlca
	out (0x30),a
	ret

__ugetc:
	ld a,#0x81
	out (0x38),a
	rlca
	out (0x30),a
	ld l,(hl)
	ld h,#0
	jr uout

__ugetw:
	ld a,#0x81
	out (0x38),a
	rlca
	out (0x30),a
	ld a,(hl)
	inc hl
	ld h,(hl)
	ld l,a
	jr uout
	
__uputc:
	pop bc
	pop de
	pop hl
	push hl
	push de
	push bc
	ld a,#0x81
	out (0x38),a
	rlca
	out (0x30),a
	ld (hl),e
	jr uout

__uputw:
	pop bc
	pop de
	pop hl
	push hl
	push de
	push bc
	ld a,#0x81
	out (0x38),a
	rlca
	out (0x30),a
	ld (hl),e
	inc hl
	ld (hl),d
	jr uout
