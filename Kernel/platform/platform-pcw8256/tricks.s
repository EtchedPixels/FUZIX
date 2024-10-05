        .module tricks

        .include "kernel.def"
        .include "../../cpu-z80/kernel-z80.def"

	.include "../../lib/z80bank16kfc-core.s"

;
;	We are copying from udata (DE) to process tables (HL)
;

	.globl bankfork

bankfork:
	ld de,#_udata + U_DATA__U_PAGE
	ld a,(de)
	ld c,(hl)
	ld b,a
	call copy16k
	inc de
	inc hl
	ld a,(de)
	ld c,(hl)
	cp b
	; Don't copy if the same page code as before
	; Will need refining when we do stack smart stuff
	ld b,a
	call nz, copy16k
	inc de
	inc hl
	ld a,(de)
	ld c,(hl)
	cp b
	; If the top is a copy we are done
	ret z
	; Fall through
copy16k:
	; 0x4000 = source
	; 0x8000 = target
	out (0xF1),a
	ld a,c
	out (0xF2),a
	; This is ok our stack is in common
	push bc
	push de
	push hl
	ld hl,#0x4000
	ld de,#0x8000
	ld bc,#0x4000
	ldir		; optimise me ?
	pop hl
	pop de
	pop bc
	; Undo our remapping
	jp map_kernel
