;
;	No fancy optimisations here. It's not clear they are worth it once
; 	you have to deal with misaligned transfers that might cross source
;	or destination banks. One optimisation might be worth doing - that
; 	is 512 byte blocks that don't bank cross ?
;
        .module usermem

	.include "kernel.def"
        .include "../../cpu-z80/kernel-z80.def"

        ; exported symbols
        .globl __uget
        .globl __ugetc
        .globl __ugetw

	.globl outcharhex
	.globl outhl

        .globl __uput
        .globl __uputc
        .globl __uputw
        .globl __uzero

	.globl  map_proc_save
	.globl  map_kernel_restore
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
	call map_proc_save
	ld (hl), e
uputc_out:
	jp map_kernel_restore			; map the kernel back below common

__uputw:
	pop iy
	pop bc	;	return
	pop de	;	word
	pop hl	;	dest
	push hl
	push de
	push bc
	push iy
	call map_proc_save
	ld (hl), e
	inc hl
	ld (hl), d
	jp map_kernel_restore

__ugetc:
	call map_proc_save
        ld l, (hl)
	ld h, #0
	jp map_kernel_restore

__ugetw:
	call map_proc_save
        ld a, (hl)
	inc hl
	ld h, (hl)
	ld l, a
	jp map_kernel_restore

__uput:
	push ix
	ld ix, #0
	add ix, sp
	call uputget			; source in HL dest in DE, count in BC
	jr z, uput_out			; but count is at this point magic
	
	call map_proc_save

uput_l:	ld a, (hl)
	inc hl
	ld (de), a
	inc de
	djnz uput_l
	dec c
	jr nz, uput_l

uput_unmap:
	call map_kernel_restore

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

	call map_proc_save
	
uget_l:
	ld a, (hl)
	inc hl
	ld (de), a
	inc de
	djnz uget_l
	dec c
	jr nz, uget_l
	jr uput_unmap

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
	call map_proc_save
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
