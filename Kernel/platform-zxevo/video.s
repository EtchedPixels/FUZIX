;
;        ZX Evolution Text Mode
;
;	Not particularly sane
;
;	80 column but split so
;	0x01C0 holds 40 chars then a gap (left of each pair)
;	0x11C0 holds the same for the right pairs
;	0x21C0 is the attributes of the *right* pairs
;	0x31C0 is the attributes of the left pairs
;
;	And the attributes are
;
;	bits 7/5/4/3	= pixel colour
;	bits 6/2/1/0	= background colour
;
;	Lower font is ascii, upper font is russian but font can be
;	changed providing you jump through some weird hoops
;

        ; exported symbols
        .globl _plot_char
        .globl _scroll_down
        .globl _scroll_up
        .globl _cursor_on
        .globl _cursor_off
	.globl _cursor_disable
        .globl _clear_lines
        .globl _clear_across
        .globl _do_beep
	.globl _fontdata_8x8
	.globl _curattr
	.globl _vtattr

	; imports

	.globl map_video
	.globl map_kernel
;
;	Y * 64 + X / 2
;	Base is then 0x1C0 (left) 0x11C0 (right)
;
videopos:
	ld a,e			; Get Y (0 - 24)
	ld e,d			; X
	rl e			; So it shifts back as we want
	rra
	srl e
	rra			; In effect Y x 64 +
	srl e			; A,E is now the 16bit offset
				; Y * 64 + x / 2
	ld d,a
	jr nc, left
	set 4,d			; right
left:
	ld hl,#0x01c0
	add hl,de
	ld a,h
	xor #0x30
	ld d,a			; attributes
	ld e,l			;
	ret

_plot_char:
        pop hl
        pop de              ; D = x E = y
        pop bc
        push bc
        push de
        push hl

	call map_video
        call videopos
        ld (hl), a
	ld a,(_curattr)
        ld (de), a
	jp map_kernel

_clear_lines:
        pop hl
        pop de              ; E = line, D = count
        push de
        push hl
	; This way we handle 0 correctly
	inc d
	ld c,d
	ld d,#0
	call videopos
	ld a,(_curattr)
	call map_video
	jr nextline

clear_next_line:
	ld b,#80
clear_line_loop:
	ld (hl),#' '
	inc hl
	ld (de),a
	inc de
	djnz clear_line_loop
nextline:
        dec c
        jr nz, clear_next_line
	jp map_kernel

_clear_across:
        pop hl
        pop de              ; DE = coords 
        pop bc              ; C = count
        push bc
        push de
        push hl
	ld a,c
	or a
	ret z		    ; No work to do - bail out
        call videopos
	ld a,(_curattr)
	ld b,c
	call map_video
clear_loop:
	ld (hl),#' '
	inc hl
	ld (de),a
	inc de
	djnz clear_loop
	jp map_kernel

_scroll_down:
	call map_video
	ld hl,#0x07BF
	ld de,#0x07FF
	ld bc,#1600-64
	lddr
	ld hl,#0x17BF
	ld de,#0x17FF
	ld bc,#1600-64
	lddr
	ld hl,#0x27BF
	ld de,#0x27FF
	ld bc,#1600-64
	ldir
	ld hl,#0x37BF
	ld de,#0x37FF
	ld bc,#1600-64
	ldir
	jp map_kernel

_scroll_up:
	call map_video
	ld hl,#0x01E0
	ld de,#0x01C0
	ld bc,#1600-64
	ldir
	ld hl,#0x11E0
	ld de,#0x11C0
	ld bc,#1600-64
	ldir
	ld hl,#0x21E0
	ld de,#0x21C0
	ld bc,#1600-64
	ldir
	ld hl,#0x31E0
	ld de,#0x31C0
	ld bc,#1600-64
	ldir
	jp map_kernel

_cursor_on:
        pop hl
        pop de
        push de
        push hl
        ld (cursorpos), de
_cursor_disable:
_cursor_off:
	ld de,(cursorpos)
        call videopos
	call map_video
	ld a,(de)
	xor #0x3F
	ld (de),a
	jp map_kernel

_do_beep:
        ret

        .area _DATA

cursorpos:
        .dw 0

	.area _COMMONMEM
_curattr:
	.db 7
