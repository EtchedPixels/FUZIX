            .module vdp

            .include "kernel.def"
            .include "../kernel-z80.def"


	    .globl _outputtty
	    .globl _scroll_up
	    .globl _scroll_down
	    .globl _clear_across
	    .globl _clear_lines
	    .globl _cursor_off
	    .globl _cursor_on
	    .globl _cursor_disable
	    .globl _plot_char
	    .globl _vtattr_notify
	    .globl _fontdata_6x8
	    .globl vdp_save_romfont
;
;	Don't provide the global vt hooks in vdp1.s, we want to wrap them
;	for our dual monitor setup
;
VDP_DIRECT   .equ	0
VDP_IRQ	     .equ	0	; leave the vdp irq off

;	Dummy for the vdp1.s code. We actually use the ROM font

_fontdata_6x8:

;
;	On an MSX at 4Mhz our loop worst case is 26 clocks so for
;	graphics one we need a nop
;
.macro VDP_DELAY
	    nop
.endm
.macro VDP_DELAY2
	    nop
.endm

	    .area _DISCARD
vdp_copych:
	    ld b,#8
copy1:
	    out (c),l
	    out (c),h
	    VDP_DELAY
	    inc hl
	    in a,(1)
	    VDP_DELAY
	    out (c),e
	    out (c),d
	    out (1),a
	    inc de
	    djnz copy1
	    ret
vdp_save_romfont:
	    ld hl,#0x1800
	    ld de,#0x7c00	; 3C00 in write mode
	    ld bc,#0x8002	; 128 chars, port 2
vdp_fontcopy:
	    call vdp_copych
	    bit 7,d
	    jr z,vdp_fontcopy	; DE = 0x8000 we are done
	    ret

	    .area _COMMONMEM

	    .include "../dev/vdp1.s"

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

_cursor_on:
	     ld a, (_outputtty)
	     cp #4
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

_cursor_off: ld a, (_outputtty)
	     cp #4
	     jp nz, cursor_off		; VDP
_cursor_disable:
	     ret

_clear_across:
	     ld a, (_outputtty)
	     cp #4
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
	     ld a, (_outputtty)
	     cp #4
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

_scroll_up:
	     ld a, (_outputtty)
	     cp #4
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
	     ld a, (_outputtty)
	     cp #4
	     jp nz, scroll_down
	     ld hl, (crtcbase)
	     ld de, #0xFF80
	     jr do_scroll

_plot_char:
	     ld a, (_outputtty)
	     cp #4
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
_vtattr_notify:
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
