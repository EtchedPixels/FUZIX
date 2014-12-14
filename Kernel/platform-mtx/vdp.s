            .module vdp

            .include "kernel.def"
            .include "../kernel.def"

	    .include "../dev/vdp1.s"

	    .globl _curtty
	    .globl _scroll_up
	    .globl _scroll_down
	    .globl _clear_across
	    .globl _clear_lines
	    .globl _cursor_off
	    .globl _cursor_on
	    .globl _plot_char


;
;	Don't provide the global vt hooks in vdp1.s, we want to wrap them
;	for our dual monitor setup
;
VDP_DIRECT   equ	0

	    .area _COMMONMEM

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
	     ret

_clear_across:
	     ld a, (_curtty)
	     or a
	     jp nz, clear_across
	     ret

_clear_lines:
	     ld a, (_curtty)
	     or a
	     jp nz, clear_lines
	     ret

_scroll_up:  ld a, (_curtty)
	     or a
	     jp nz, scroll_up
	     ret

_scroll_down:
	     ld a, (_curtty)
	     or a
	     jp nz, scroll_down
	     ret

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
;
;	FIXME: should use vdpport, but right now vdpport is in data not
;	common space.
;
platform_interrupt_all:
	    ret
