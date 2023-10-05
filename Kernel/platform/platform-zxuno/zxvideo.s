;
;        Timex VT primitives
;
;	512 pixel mode but on the uno we have it mapped into
;	bank 7 which is not normally mapped
;

        .module tmxvideo

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
	.globl map_kernel_restore
	.globl map_video_save

        .area _COMMONMEM

map_videopos:
	call map_video_save
videopos:
	srl d		    ; we alternate pixels between two screens
	push af
        ld a,e
        and #7
        rrca
        rrca
        rrca 
        add a,d
        ld d,e
        ld e,a
	pop af
	jr c, rightside
        ld a,d
        and #0x18
        or #0xC0	    ; Left screen
        ld d,a
	ret
rightside:
        ld a,d
        and #0x18
        or #0xE0	    ; Left screen
        ld d,a
	ret

_plot_char:
        pop hl
        pop de              ; D = x E = y
        pop bc
        push bc
        push de
        push hl

	ld hl,(_vtattr)	    ; l is vt attributes
	push hl		    ; save attributes as inaccessible once vid mapped

        call map_videopos

        ld b, #0            ; calculating offset in font table
        ld a, c

	or a		    ; clear carry
        rla
        rl b
        rla
        rl b
        rla
        rl b
        ld c, a

        ld hl, #_fontdata_8x8    ; font
        add hl, bc          ; hl points to first byte of char data

	pop bc
	; We do underline for now - not clear italic or bold are useful
	; with the font we have.

        ; printing
plot_char_loop:
        ld a, (hl)
        ld (de), a
        inc hl              ; next byte of char data
        inc d               ; next screen line

        ld a, (hl)
        ld (de), a
        inc hl              ; next byte of char data
        inc d               ; next screen line

        ld a, (hl)
        ld (de), a
        inc hl              ; next byte of char data
        inc d               ; next screen line

        ld a, (hl)
        ld (de), a
        inc hl              ; next byte of char data
        inc d               ; next screen line

        ld a, (hl)
        ld (de), a
        inc hl              ; next byte of char data
        inc d               ; next screen line

        ld a, (hl)
        ld (de), a
        inc hl              ; next byte of char data
        inc d               ; next screen line

        ld a, (hl)
        ld (de), a
        inc hl              ; next byte of char data
        inc d               ; next screen line

        ld a, (hl)
	bit 1,c		    ; underline ?
	jr nz, last_ul
plot_attr:
        ld (de), a
	jp map_kernel_restore

last_ul:
	ld a,#0xff
	jr plot_attr

_clear_lines:
        pop hl
        pop de              ; E = line, D = count
        push de
        push hl
	; This way we handle 0 correctly
	inc d
	jr nextline

clear_next_line:
        push de
        ld d, #0            ; from the column #0
        ld b, d             ; b = 0
        ld c, #64           ; clear 64 cols
        push bc
        push de
        call _clear_across
        pop hl              ; clear stack
        pop hl

        pop de
        inc e
nextline:
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
	ld a,c
	or a
	ret z		    ; No work to do - bail out
        call map_videopos       ; first pixel line of first character in DE

	; Save it in HL
	ld h,d
	ld l,e

        xor a

	; TODO Figure out if starting L or R
	bit 5,d
	jr nz, clear_start_r


        ; no boundary checks. Assuming that D + C < SCREEN_WIDTH
clear_line:

	; Do the left character
        ld b, #8            ; 8 pixel lines to clear for this char
clear_char_l:
        ld (de), a
        inc d
        djnz clear_char_l

	; Left was final char
	dec c
	jr z, lines_done

	; Recover position from HL and move 8K up
	ld d,h
	ld e,l
	set 5,d

clear_start_r:
        ld b, #8            ; 8 pixel lines to clear for this char
clear_char_r:
        ld (de), a
        inc d
        djnz clear_char_r

        inc hl
	ld d,h
	ld e,l

	; Next character ?
        dec c
        jr nz, clear_line
lines_done:
	jp map_kernel_restore

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
        djnz copy_pixel_line

        pop de
        pop hl
	push hl
	push de
	set 5,d		; Even half
	set 5,h
	ld b,#32
copy_pixel_line_r:
	ld a,(hl)
	ld (de),a
	inc e
	inc l
	djnz copy_pixel_line_r
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

	call map_video_save

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
        jp map_kernel_restore


_scroll_up:
        ; set HL = (0,1), DE = (0, 0)
        xor a
        ld d, a
        ld e, a
        ld h, a
        ld l, #1
        ld c, #23           ; 23 lines to move

	call map_video_save

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
	jp map_kernel_restore

_cursor_on:
        pop hl
        pop de
        push de
        push hl
        ld (cursorpos), de

        call map_videopos
        ld a, #7
        add a, d
        ld d, a
        ld a, #0xFF
        ld (de), a
        jp map_kernel_restore
_cursor_disable:
_cursor_off:
        ld de, (cursorpos)
        call map_videopos
        ld a, #7
        add a, d
        ld d, a
        xor a
        ld (de), a
	jp map_kernel_restore

_do_beep:
        ret

        .area _COMMONDATA

cursorpos:
        .dw 0

_curattr:
	.db 7

