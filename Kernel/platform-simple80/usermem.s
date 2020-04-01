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
	.globl sio_reg
	.globl bits_to_user
	.globl bits_from_user

	.globl _int_disabled

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
	di
	ld a,(sio_reg)
	out (0x03),a		; Port B WR 1
	ld a,(bits_to_user)
	out (0x03),a		; port B WR 1 to 0x58
	ld (hl),#0
	ld a,(_int_disabled)
	or a
	jr nz, noteiz
	ei
noteiz:
	dec bc
	ld a,b
	or c
	jr z, uout
	ld e,l
	ld d,h
	inc de
	ldir
uout:
	di
	ld a,(sio_reg)		; port B WR 1
	out (0x03),a
	ld a,(bits_from_user)
	out (0x03),a		; back to kernel
	ld a,(_int_disabled)
	or a
	ret nz
	ei
	ret

__ugetc:
	di
	ld a,(sio_reg)
	out (0x03),a
	ld a,(bits_to_user)
	out (0x03),a
	ld l,(hl)
	ld h,#0
	jr uout

__ugetw:
	di
	ld a,(sio_reg)
	out (0x03),a
	ld a,(bits_to_user)
	out (0x03),a
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
	di
	ld a,(sio_reg)
	out (0x03),a
	ld a,(bits_to_user)
	out (0x03),a
	ld (hl),e
	jr uout

__uputw:
	pop bc
	pop de
	pop hl
	push hl
	push de
	push bc
	di
	ld a,(sio_reg)
	out (0x03),a
	ld a,(bits_to_user)
	out (0x03),a
	ld (hl),e
	inc hl
	ld (hl),d
	jr uout
