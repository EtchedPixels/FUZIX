;
;        zx128 vt primitives
;

        .module zxvideo

        ; exported symbols
        .globl _plot_char
        .globl _scroll_down
        .globl _scroll_up
        .globl _cursor_on
        .globl _cursor_off
        .globl _clear_lines
        .globl _clear_across
        .globl _do_beep
	.globl _vtattr_notify

        .area _VIDEO

        ; colors are ignored everywhere for now

videopos:
        ld a,e
        and #7
        rrca
        rrca
        rrca 
        add a,d
        ld d,e
        ld e,a
        ld a,d
        and #0x18
        or #0xC0	    ; not 0x40 as in screen 7
        ld d,a
        ret

_plot_char:
	pop iy
        pop hl
        pop de              ; D = x E = y
        pop bc
        push bc
        push de
        push hl
	push iy

        call videopos

	;
	;	TODO: Map char 0x60 to a grave accent bitmap rather
	;	than fudging with a quote
	;

        ld b, #0            ; calculating offset in font table
        ld a, c
	cp #0x60
	jr nz, nofiddle
	ld a, #0x27
nofiddle:
	or a		    ; clear carry
        rla
        rl b
        rla
        rl b
        rla
        rl b
        ld c, a

        ld hl, #0x3C00	    ; ROM font
        add hl, bc          ; hl points to first byte of char data


        ; printing
        ld c, #8
plot_char_loop:
        ld a, (hl)
        ld (de), a
        inc hl              ; next byte of char data
        inc d               ; next screen line
        dec c
        jr nz, plot_char_loop
        ret


_clear_lines:
	pop bc
        pop hl
        pop de              ; E = line, D = count
        push de
        push hl
	push bc

clear_next_line:
        push de
        ld d, #0            ; from the column #0
        ld b, d             ; b = 0
        ld c, #32           ; clear 32 cols
        push bc
        push de
	push af
        call _clear_across
	pop af
        pop hl              ; clear stack
        pop hl

        pop de
        inc e
        dec d
        jr nz, clear_next_line

        ret


_clear_across:
	pop iy
        pop hl
        pop de              ; DE = coords 
        pop bc              ; C = count
        push bc
        push de
        push hl
	push iy
        call videopos       ; first pixel line of first character in DE
        push de
        pop hl              ; copy to hl
        xor a

        ; no boundary checks. Assuming that D + C < SCREEN_WIDTH

clear_line:
        ld b, #8            ; 8 pixel lines to clear for this char
clear_char:
        ld (de), a
        inc d
        dec b
        jr nz, clear_char

        ex de, hl
        inc de
        push de
        pop hl

        dec c
        jr nz, clear_line
        ret

copy_line:
        ; HL - source, DE - destination

        ; convert line coordinates to screen coordinates both for DE and HL
        push de
        ex de, hl
        call videopos
        ex de, hl
        pop de
        call videopos

        ld c, #8

copy_line_nextchar:
        push hl
        push de

        ld b, #32

copy_pixel_line:
        ld a, (hl)
        ld (de), a
        inc e
        inc l
        dec b
        jr nz, copy_pixel_line

        pop de
        pop hl
        inc d
        inc h
        dec c
        jr nz, copy_line_nextchar
        ret

        ; TODO: the LDIR way should be much faster

_scroll_down:
        ; set HL = (0,22), DE = (0, 23)
        xor a
        ld d, a
        ld h, a
        ld l, #22
        ld e, #23
        ld c, #23           ; 23 lines to move

loop_scroll_down:
        push hl
        push de
        push bc

        call copy_line

        pop bc
        pop de
        pop hl

        dec l
        dec e
        dec c
        jr nz, loop_scroll_down

        ret


_scroll_up:
        ; set HL = (0,1), DE = (0, 0)
        xor a
        ld d, a
        ld e, a
        ld h, a
        ld l, #1
        ld c, #23           ; 23 lines to move

loop_scroll_up:
        push hl
        push de
        push bc

        call copy_line

        pop bc
        pop de
        pop hl

        inc l
        inc e
        dec c
        jr nz, loop_scroll_up

        ret

_cursor_on:
	pop bc
        pop hl
        pop de
        push de
        push hl
	push bc
        ld (cursorpos), de

        call videopos
        ld a, #7
        add a, d
        ld d, a
        ld a, #0xFF
        ld (de), a
        ret
_cursor_off:
        ld de, (cursorpos)
        call videopos
        ld a, #7
        add a, d
        ld d, a
        xor a
        ld (de), a
_vtattr_notify:
        ret

        ; FIXME: now this is_do_silent_click actually
_do_beep:
        ld e, #0xFF         ; length
        ld c, #0xFE         ; beeper port
        ld l, #0x10         ; beeper bit
loop_beep:
        ld a, l
        out (c), a
        xor a
        out (c), a
        dec bc
        ld a, b
        or c
        jr nz, loop_beep
        ret

        .area _DATA

cursorpos:
        .dw 0
