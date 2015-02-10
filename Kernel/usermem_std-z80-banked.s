;
;	No fancy optimisations here. It's not clear they are worth it once
; 	you have to deal with misaligned transfers that might cross source
;	or destination banks. One optimisation might be worth doing - that
; 	is 512 byte blocks that don't bank cross ?
;
        .module usermem

	.include "platform/kernel.def"
        .include "kernel.def"

        ; exported symbols
        .globl __uget
        .globl __ugetc
        .globl __ugets
        .globl __ugetw

	.globl outcharhex
	.globl outhl

        .globl __uput
        .globl __uputc
        .globl __uputw
        .globl __uzero

	.globl  map_process_always
	.globl  map_kernel
;
;	We need these in common as they bank switch
;
        .area _COMMONMEM

uputget:
        ; load DE with the byte count
        ld e, 10(ix) ; byte count
        ld d, 11(ix)
	ld a, d
	or e
	ret z		; no work
	dec de		; we return BC as a count for two 8bit loops
	ld b, e		; not a 16bit value
	inc b		; See http://map.grauw.nl/articles/fast_loops.php
	inc d
	ld c, d
        ; load HL with the source address
        ld l, 6(ix) ; src address
        ld h, 7(ix)
        ; load DE with destination address (in userspace)
        ld e, 8(ix)
        ld d, 9(ix)
	ld a, b
	or c
	ret

__uputc:
	pop iy	;	bank
	pop bc	;	return
	pop de	;	char
	pop hl	;	dest
	push hl
	push de
	push bc
	push iy
	call map_process_always
	ld (hl), e
uputc_out:
	jp map_kernel			; map the kernel back below common

__uputw:
	pop iy
	pop bc	;	return
	pop de	;	word
	pop hl	;	dest
	push hl
	push de
	push bc
	push iy
	call map_process_always
	ld (hl), e
	inc hl
	ld (hl), d
	jp map_kernel

__ugetc:
	pop de
	pop bc	; return
	pop hl	; address
	push hl
	push bc
	push de
	call map_process_always
        ld l, (hl)
	ld h, #0
	jp map_kernel

__ugetw:
	pop de
	pop bc	; return
	pop hl	; address
	push hl
	push bc
	push de
	call map_process_always
        ld a, (hl)
	inc hl
	ld h, (hl)
	ld l, a
	jp map_kernel

__uput:
	push ix
	ld ix, #0
	add ix, sp
	call uputget			; source in HL dest in DE, count in BC
	jr z, uput_out			; but count is at this point magic
	
uput_l:	ld a, (hl)
	inc hl
	call map_process_always
	ld (de), a
	call map_kernel
	inc de
	djnz uput_l
	dec c
	jr nz, uput_l

uput_out:
	pop ix
	ld hl, #0
	ret


__uget:
	push ix
	ld ix, #0
	add ix, sp
	call uputget			; source in HL dest in DE, count in BC
	jr z, uput_out			; but count is at this point magic
	
uget_l:
	call map_process_always
	ld a, (hl)
	inc hl
	call map_kernel
	ld (de), a
	inc de
	djnz uget_l
	dec c
	jr nz, uget_l
	jr uput_out

__ugets:
	push ix
	ld ix, #0
	add ix, sp
	call uputget			; source in HL dest in DE, count in BC
	jr z, ugets_bad			; but count is at this point magic
	
ugets_l:
	call map_process_always
	ld a, (hl)
	inc hl
	call map_kernel
	ld (de), a
	or a
	jr z, ugets_good
	inc de
	djnz ugets_l
	dec c
	jr nz, ugets_l
	dec de
	xor a
	ld (de), a
ugets_bad:
	ld hl,  #0xFFFF			; flag an error
	jr uput_out
ugets_good:
	ld hl,#0
	jr uput_out

;
__uzero:
	pop iy
	pop de	; return
	pop hl	; address
	pop bc	; size
	push bc
	push hl	
	push de
	push iy
	ld a, b	; check for 0 copy
	or c
	ret z
	call map_process_always
	ld (hl), #0
	dec bc
	ld a, b
	or c
	jp z, uputc_out
	ld e, l
	ld d, h
	inc de
	ldir
	jp uputc_out
