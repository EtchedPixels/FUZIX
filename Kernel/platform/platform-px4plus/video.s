
		.module video

		.include "kernel.def"

		.globl _scroll_up
		.globl _scroll_down
		.globl _plot_char
		.globl _clear_lines
		.globl _clear_across
		.globl _cursor_on
		.globl _cursor_off
		.globl _do_beep

		.globl _fontdata_8x8

		; Just as its handy in the map file
		.globl cursorpos

		.area	_CODE
_scroll_up:	ld a, (yoff)
		add #8		; for now with 8x8 fonts
_scroll_ud:	and #0xBF
		ld (yoff), a
		out (0x09), a
		rra		; Compute the line bias
		rra
		rra
		and #7
		ld (ybias), a	; For character positioning
		ret
_scroll_down:	ld a, (yoff)
		sub #8
		jr _scroll_ud

;
;	Turn a co-ordinate pair in DE into an address in DE
;
addr_de:
		ld a, (ybias)
		; 32 bytes/line so with 8 char rows
		; 256 bytes per char line
		add d
		and #7		; 8 rows 0-7 wrapping
		add #VIDBASE	; add video base
		ld d, a
		ret		; E is already correct as 0-31 are the top
				; scan lines

_clear_lines:
		pop bc
		pop hl
		pop de
		push de
		push hl
		push bc
		; E = y, D = count
		ld b, d
		ld d, #0
clloop:		push de
		push bc
		call addr_de
		ld h, d
		ld l, e
		ld (hl), #0
		inc de
		ld bc, #255
		ldir
		pop bc
		pop de
		inc e
		djnz clloop
		ret
_plot_char:
		pop ix
		pop hl
		pop de
		pop bc
		push bc
		push de
		push hl
		push ix
		call addr_de
		ld l, c
		ld h, #0
		add hl, hl
		add hl, hl
		add hl, hl
		ld bc, #_fontdata_8x8
		add hl, bc
		ex de, hl
		ld bc, #32
		ld a, (de)	; Row 0
		inc de
		ld (hl), a
		add hl, bc
		ld a, (de)	; Row 1
		inc de
		ld (hl), a
		add hl, bc
		ld a, (de)	; Row 2
		inc de
		ld (hl), a
		add hl, bc
		ld a, (de)	; Row 3
		inc de
		ld (hl), a
		add hl, bc
		ld a, (de)	; Row 4
		inc de
		ld (hl), a
		add hl, bc
		ld a, (de)	; Row 5
		inc de
		ld (hl), a
		add hl, bc
		ld a, (de)	; Row 6
		inc de
		ld (hl), a
		add hl, bc
		ld a, (de)	; Row 7
		ld (hl), a
		ret

_clear_across:
		pop ix
		pop hl
		pop de		; Co-ordinates
		pop bc
		push bc		; Count
		push de
		push hl
		push ix
		call addr_de
		ld b,c
		ld c, #8
		ex de, hl
		xor a
clearalo:	push bc
		push hl
clearal:
		ld (hl), a
		inc hl
		djnz clearal
		pop hl
		ld bc, #32
		add hl, bc
		pop bc
		dec c
		jr nz, clearalo
		ret

_cursor_on:
		pop bc
		pop hl
		pop de
		push de
		push hl
		push bc
		ld (cursorpos), de
cursordo:	call addr_de
		ex de, hl
		ld bc, #32
		ld e, #8
cursorl:	ld a, (hl)
		cpl
		ld (hl), a
		add hl, bc
		dec e
		jr nz, cursorl
		ret

_cursor_off:
		ld de, (cursorpos)
		jr cursordo

_do_beep:
		ret

		.area _DATA

cursorpos:	.dw 0
;
;	Shadow copy of GAPNDL registers we care about
;
yoff:		.db 0
ybias:		.db 0

