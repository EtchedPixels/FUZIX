;
;        zx128 vt primitives

        .module zx128

        ; exported symbols
        .globl _plot_char
        .globl _scroll_down
        .globl _scroll_up
        .globl _cursor_on
        .globl _cursor_off
        .globl _clear_lines
        .globl _clear_across
        .globl _do_beep

        .globl _fontdata_8x8

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
        or #0x40
        ld d,a
        ret

_plot_char:
        pop hl
        pop de              ; D = x E = y
        pop bc
        push bc
        push de
        push hl

        call videopos

        ld b, #0            ; calculating offset in font table
        ld a, c
        rla
        rl b
        rla
        rl b
        rla
        rl b
        ld c, a

        ld hl, #_fontdata_8x8
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
        pop hl
        pop de              ; E = line, D = count
        push de
        push hl

clear_next_line:
        push de
        ld d, #0            ; from the column #0
        ld b, d             ; b = 0
        ld c, #32           ; clear 32 cols
        push bc
        push de
        call _clear_across

        pop hl              ; clear stack
        pop hl

        pop de
        inc e
        dec d
        jr nz, clear_next_line

        ret


_clear_across:
        pop hl
        pop de              ; DE = coords 
        pop bc              ; C = count
        push bc
        push de
        push hl
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
        ; set HL = (0,0), DE = (0, 1)
        xor a
        ld d, a
        ld h, a
        ld l, a
        ld e, #1
        ld c, #23           ; 23 lines to move

loop_scroll_down:
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
        jr nz, loop_scroll_down

        ret


_scroll_up:
        ; set HL = (0,23), DE = (0, 22)
        xor a
        ld d, a
        ld h, a
        ld l, #23
        ld e, #22
        ld c, #23           ; 23 lines to move

loop_scroll_up:
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
        jr nz, loop_scroll_up

        ret

_cursor_on:
        pop hl
        pop de
        push de
        push hl
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
