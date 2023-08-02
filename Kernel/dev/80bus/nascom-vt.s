;
;	The Nascom has a fairly simple default video console except
;	that each line has margins of space that are used for other
;	stuff
;
		.module nascom-vt


		.globl _cursor_off
		.globl _cursor_disable
		.globl _cursor_on
		.globl _plot_char
		.globl _clear_lines
		.globl _clear_across
		.globl _vtattr_notify
		.globl _scroll_up
		.globl _scroll_down


		.include "kernel.def"

		.area _CODE
;
;	Required to preserve BC
;
addr:		
		ld a,e		; get Y
		dec a		; weird line wrapping
		and #15
		add a,a
		add a,a
		add a,a
		add a,a		; x16: as far as we can 8bit add
		ld l,a
		ld h,#0
		ld e,d
		ld d,h		; DE is now 00xx where xx is the X value
		add hl,hl	; x 32
		add hl,hl	; x 64
		add hl,de
		ld de,#VIDEO+10
		add hl,de
		ret

_cursor_off:
		ld hl,(cpos)
		bit 7,h		; all valid cpos values are > 0x8000
		ret z
		ld a,(csave)
		ld (hl),a
		xor a
		ld (cpos+1),a
_cursor_disable:
		ret

_cursor_on:
		pop hl
		pop de
		push de
		push hl
		call addr
		ld a,(hl)
		ld (hl),#'_;
		ld (csave),a
		ld (cpos),hl
		ret

_plot_char:
		pop hl
		pop de
		pop bc
		push bc
		push de
		push hl
		call addr
		ld (hl),c
		ret

;
;	The weird layout means we can't do a single ldir
;
_clear_lines:
		pop hl
		pop de
		push de		; E = Y D = count
		push hl
		ld c,d
		ld d,#0
		call addr
		ld a,c		; A is now line count
		or a
		ret z
wipeline:
		ld b,#48
wiper:		ld (hl),#' '
		inc hl
		djnz wiper
		ld bc,#16
		add hl,bc
		dec a
		jr nz, wipeline
		ret

_clear_across:
		pop hl
		pop de		; E = y D = x
		pop bc		; C = count
		push bc
		push de
		push hl
		call addr
		ld a,#' '
		ld b,c
clear2:		ld (hl),a
		inc hl
		djnz clear2
_vtattr_notify:
		ret

_scroll_up:
		; Do the special case first
		ld hl,#0xF80A
		ld de,#0xFBCA
		ld bc,#48
		ldir
		ld de,#0xF80A
		ld hl,#0xF84A
		; BC is always 0 here
		ld a,#15
nextup:
		ld c,#48
		ldir
		ld c,#16
		add hl,bc
		ex de,hl
		add hl,bc
		ex de,hl
		dec a
		jr nz,nextup
		ret

_scroll_down:
		ld hl,#0xFB79
		ld de,#0xFBB9
		ld a,#15
nextdown:
		ld bc,#48
		lddr
		ld bc,#0xFFF0		; - 16
		add hl,bc
		ex de,hl
		add hl,bc
		ex de,hl
		dec a
		jr nz, nextdown
		; Top line special case
		ld hl,#0xFBC0
		ld de,#0xF800
		ld bc,#48
		ldir
		ret

		.area _DATA
csave:		.byte 0
cpos:		.word 0
