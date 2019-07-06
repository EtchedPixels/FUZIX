            .module vdp

            .include "kernel.def"
            .include "../kernel-z80.def"

	    .include "../dev/vdp1.s"

	    .globl _curtty
	    .globl _scroll_up
	    .globl _scroll_down
	    .globl _clear_across
	    .globl _clear_lines
	    .globl _cursor_off
	    .globl _cursor_on
	    .globl _cursor_disable
	    .globl _plot_char
	    .globl vdpload

	    .globl _fontdata_6x8
;
;	Don't provide the global vt hooks in vdp1.s, we want to wrap them
;	for our dual monitor setup
;
VDP_DIRECT   .equ	0

	    .area _COMMONMEM

;
;	Font uploader
;
vdpload:
	     ld hl, #_fontdata_6x8
	     ; Remember the first 32 symbols (256 bytes) are unused
	     ld de, #0x4000 + 0x1900 	; write to 0x4000 + fontbase
	     ld bc, (_vdpport)		; port
	     ld b, #0			; but we want 256 not the rows
	     ld a, #3
vdploadl:
	     push af
vdploadl2:
	     out (c), e		; select target
	     out (c), d
	     dec c		; write font byte
	     ld a, (hl)
	     rlca		; font packed left
	     rlca
	     out (c), a
	     inc c
	     inc de
	     inc hl
	     djnz vdploadl2	; 256 bytes done
	     pop af
	     dec a
	     jr nz, vdploadl	; 768 bytes total

	     ld de,#0x1800	; 24 lines from 0
	     push de
	     call clear_lines
	     pop de
	     ret


;
;	Turn co-ordinates D,E into offset HL
;
videopos6:
	     ld a, e
	     add a
	     add a
	     add a			; x 8 (max 24 x 8 - 192)
	     ld l, a
	     ld h, #0
	     add hl, hl			; x 16
	     push hl
	     add hl, hl			; x 32
	     add hl, hl			; x 64
	     ld a, d
	     pop de			; + x 16 = x 80
	     add hl, de
	     ld e, a
	     ld d, #0
	     add hl, de			; and X
	     ld de, (crtcbase)		; adjust for CRTC base
	     add hl, de
	     ld a, h
	     and #7			; wrap
	     ld h, a
	     ret
;
;	This is a bit different as we support both the vdp1 and the 6845
; port
;

_cursor_on:  ld a, (_curtty)
	     or a
	     jp nz,  cursor_on		; VDP
	     pop hl
	     pop de
	     push de
	     push hl
	     call videopos6
	     ld a, #14
	     ld c, #0x39
	     out (0x38), a
	     out (c), h
	     inc a
	     out (0x38), a
	     out (c), l
	     ret

_cursor_off: ld a, (_curtty)
	     or a
	     jp nz, cursor_off		; VDP
_cursor_disable:
	     ret

_clear_across:
	     ld a, (_curtty)
	     or a
	     jp nz, clear_across
	     pop hl
	     pop de
	     pop bc
	     push bc
	     push de
	     push hl
	     call videopos6		; HL is now the offset we need
	     ld b, c
clearbyte:
	     ld a, #' '
	     out (0x32), a
	     ld a, #7
	     out (0x32), a
	     ld a, h
	     or #0xc0
	     out (0x31), a
	     ld a, l
	     out (0x30), a
	     inc hl
	     djnz clearbyte
	     ret

_clear_lines:
	     ld a, (_curtty)
	     or a
	     jp nz, clear_lines
	     pop hl
	     pop de
	     push de		; E = line, D = count
	     push hl
	     ld c,d		; Lines to copy
	     ld d, #0		; E = y, D = X
	     ld b, #80
	     call videopos6	; find our offset in HL
clearonel:
	     push bc
clearone:
	     ld a, #' '
	     out (0x32), a
	     ld a, #7
	     out (0x33), a
	     ld a, h
	     or #0xc0
	     out (0x31), a
	     ld a, l
	     out (0x30), a
	     inc hl
             djnz clearone
	     pop bc
	     dec c
	     jr nz, clearonel
	     ret

_scroll_up:  ld a, (_curtty)
	     or a
	     jp nz, scroll_up

	     ld hl, (crtcbase)
	     ld de, #80
do_scroll:
	     add hl, de
	     ld a, #12
	     out (0x38), a
	     ld a, h
	     and #0x07
	     ld h, a
	     out (0x39), a
	     ld a, #13
	     out (0x38), a
	     ld a, l
	     out (0x39), a
	     ld (crtcbase), hl
	     ret

_scroll_down:
	     ld a, (_curtty)
	     or a
	     jp nz, scroll_down
	     ld hl, (crtcbase)
	     ld de, #0xFF80
	     jr do_scroll

_plot_char:
	     ld a, (_curtty)
	     or a
	     jp nz, plot_char
	     ; Plot via 6845
	     pop hl
	     pop de			; D = x E = y
	     pop bc
	     push bc
	     push de
	     push hl
	     ld a, c			; char
	     out (0x32), a
	     ld a, #0x7			; white on black
	     out (0x33), a
	     call videopos6
	     ld a, h
	     or #0xE0			; write
	     out (0x31), a
	     ld a,l
	     out (0x30), a
	     ret


;
;	The VDP int turns up on a CTC pin and we can just
;	ignore it happily there instead of perturbing register
;	access.
;
platform_interrupt_all:
	     ret

	     .area _DATA

crtcbase:
	    .dw 0
