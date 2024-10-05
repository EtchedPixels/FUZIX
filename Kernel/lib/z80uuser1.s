;
;	Alternative user memory copiers for systems where all the kernel
;	data that is ever moved from user space is visible in the user
;	mapping.
;

        ; exported symbols
        .export __uget
        .export __ugetc
        .export __ugetw

        .export __uput
        .export __uputc
        .export __uputw

        .export __uzero

;
;	We need these in common as they bank switch
;
	.common
;
;	The basic operations are copied from the standard one. Only the
;	blk transfers are different. uputget is a bit different as we are
;	not doing 8bit loop pairs.
;
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
        ; load DE with destination address (in userspace)
        ld e, (ix + 6)
        ld d, (ix + 7)
	ret	; 	Z is still false

__uputc:
	ld hl,2
	add hl,sp
	ld e,(hl)
	inc hl
	inc hl
	ld a,(hl)
	inc hl
	ld h,(hl)
	ld l,a
	call map_proc_always
	ld (hl), e
	jp map_kernel			; map the kernel back below common

__uputw:
	ld hl,2
	add hl,sp
	ld e,(hl)
	inc hl
	ld d,(hl)	; value
	inc hl
	ld a,(hl)
	inc hl
	ld h,(hl)
	ld l,a
	call map_proc_always
	ld (hl), e
	inc hl
	ld (hl), d
	jp map_kernel

__ugetc:
	pop de
	pop hl
	push hl
	push de
	call map_proc_always
        ld l, (hl)
	ld h, 0
	jp map_kernel

__ugetw:
	pop de
	pop hl
	push hl
	push de
	call map_proc_always
        ld a, (hl)
	inc hl
	ld h, (hl)
	ld l, a
	jp map_kernel

__uput:
	push ix
	ld ix, 0
	add ix, sp
	push bc
	call uputget			; source in HL dest in DE, count in BC
	jr z, uput_out			; but count is at this point magic
	call map_proc_always
	ldir
uput_out:
	call map_kernel
	pop bc
	pop ix
	ld hl, 0
	ret

__uget:
	push ix
	ld ix, 0
	add ix, sp
	push bc
	call uputget			; source in HL dest in DE, count in BC
	jr z, uput_out			; but count is at this point magic
	call map_proc_always
	ldir
	jr uput_out

;
__uzero:
	ld hl,2
	add hl,sp
	push bc
	ld e,(hl)
	inc hl
	ld d,(hl)
	inc hl
	ld c,(hl)
	inc hl
	ld b,(hl)
	ld a, b	; check for 0 copy
	or c
	jr z, popout
	call map_proc_always
	ld l,e
	ld h,d
	ld (hl), 0
	dec bc
	ld a, b
	or c
	jr z, popout
	inc de
	ldir
popout:
	pop bc
	jp map_kernel
