;
;	No fancy optimisations here. It's not clear they are worth it once
; 	you have to deal with misaligned transfers that might cross source
;	or destination banks. One optimisation might be worth doing - that
; 	is 512 byte blocks that don't bank cross ?
;
#include "../build/kernelu.def"
#include "../cpu-z80u/kernel-z80.def"

        ; exported symbols
        .export __uget
        .export __ugetc
        .export __ugetw

        .export __uput
        .export __uputc
        .export __uputw
        .export __uzero

;	We need these in common as they bank switch
;
	.common

uputget:
        ; load DE with the byte count
        ld e, (ix + 8) ; byte count
        ld d, (ix + 9)
	ld a, d
	or e
	ret z		; no work
	dec de		; we return BC as a count for two 8bit loops
	ld b, e		; not a 16bit value
	inc b		; See http://map.grauw.nl/articles/fast_loops.php
	inc d
	ld c, d
        ; load HL with the source address
        ld l, (ix + 4) ; src address
        ld h, (ix + 5)
        ; load DE with destination address (in userspace)
        ld e, (ix + 6)
        ld d, (ix + 7)
	ld a, b
	or c
	ret

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
	jp map_kernel_restore			; map the kernel back below common

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
	call map_proc_always
	ld (hl), e
	inc hl
	ld (hl), d
	jp map_kernel_restore

__ugetc:
	pop de
	pop hl
	push hl
	push de
	call map_proc_always
        ld l, (hl)
	ld h, #0
	jp map_kernel_restore

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
	jp map_kernel_restore

__uput:
	push ix
	ld ix, #0
	add ix, sp
	push bc
	call uputget			; source in HL dest in DE, count in BC
	jr z, uput_out			; but count is at this point magic
	
uput_l:	ld a, (hl)
	inc hl
	call map_proc_always
	ld (de), a
	call map_kernel_restore
	inc de
	djnz uput_l
	dec c
	jr nz, uput_l

uput_out:
	pop bc
	pop ix
	ld hl, #0
	ret


__uget:
	push ix
	ld ix, #0
	add ix, sp
	push bc
	call uputget			; source in HL dest in DE, count in BC
	jr z, uput_out			; but count is at this point magic
	
uget_l:
	call map_proc_always
	ld a, (hl)
	inc hl
	call map_kernel_restore
	ld (de), a
	inc de
	djnz uget_l
	dec c
	jr nz, uget_l
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
	ret z
	call map_proc_always
	ld l,e
	ld h,d
	ld (hl), 0
	dec bc
	ld a, b
	or c
	jp z, done
	inc de
	ldir
done:
	pop bc
	jp map_kernel_restore
