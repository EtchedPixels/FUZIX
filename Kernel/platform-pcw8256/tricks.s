        .module tricks

        .include "kernel.def"
        .include "../kernel-z80.def"

	.include "../lib/z80fixedbank-core.s"

;
;	We are copying from process tables (HL) to udata
;
bankfork:
	ld de,#_udata + U_DATA__U_PAGE
	ld a,(de)
	ld c,(hl)
	call copy16k
	ld b,a
	inc de
	inc hl
	ld a,(de)
	ld c,(hl)
	cp b
	; Don't copy if the same page code as before
	; Will need refining when we do stack smart stuff
	call nz, copy16k
	ld b,a
	inc de
	inc hl
	ld a,(de)
	ld c,(hl)
	cp b
	; If the top is a copy we are done
	ret z
	; Fall through
copy16k:
	; 0x4000 = target
	; 0x8000 = source
	out (0xF1),a
	ld a,c
	out (0xF2),a
	; This is ok our stack is in common
	push bc
	push de
	push hl
	ld hl,#0x8000
	ld de,#0x4000
	ld bc,#0x4000
	ldir		; optimise me ?
	pop hl
	pop de
	pop bc
	; Undo our remapping
	jp map_kernel
