        .include "kernel.def"
        .include "../kernel-z80.def"

	.include "../lib/z80single.s"

;
;	The second set of very performance sensitive routines are accesses
;	to user space. We thus provide our own modified versions of these
;	for speed
;
;	This works because user space occupies 8000-FFFF and we carefully
;	pack the kernel up so that only kernel code and vectors live above
;	8000. In other words there is no case where we need to copy between
;	the high 32K of user and the high 32K of kernel. This in turn means
;	we know we can always map it all and ldir.
;

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

	.globl  map_process_always
	.globl  map_kernel
;
;	We need these in common as they bank switch
;
        .area _COMMONMEM

;
;	The basic operations are copied from the standard one. Only the
;	blk transfers are different. uputget is a bit different as we are
;	not doing 8bit loop pairs.
;
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
        ; load DE with destination address (in userspace)
        ld e, 6(ix)
        ld d, 7(ix)
	ret	; 	Z is still false

__uputc:
	pop bc	;	return
	pop de	;	char
	pop hl	;	dest
	push hl
	push de
	push bc
	call map_process_always
	ld (hl), e
uputc_out:
	jp map_kernel			; map the kernel back below common

__uputw:
	pop bc	;	return
	pop de	;	word
	pop hl	;	dest
	push hl
	push de
	push bc
	call map_process_always
	ld (hl), e
	inc hl
	ld (hl), d
	jp map_kernel

__ugetc:
	call map_process_always
        ld l, (hl)
	ld h, #0
	jp map_kernel

__ugetw:
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
	call map_process_always
	ldir
uput_out:
	call map_kernel
	pop ix
	ld hl, #0
	ret

__uget:
	push ix
	ld ix, #0
	add ix, sp
	call uputget			; source in HL dest in DE, count in BC
	jr z, uput_out			; but count is at this point magic
	call map_process_always
	ldir
	jr uput_out

;
__uzero:
	pop de	; return
	pop hl	; address
	pop bc	; size
	push bc
	push hl	
	push de
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
