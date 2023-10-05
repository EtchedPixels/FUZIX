            .module vdp

            .include "kernel.def"
            .include "../kernel-z80.def"



	    .area _COMMONMEM

	    .globl _cursor_on
	    .globl _cursor_off
	    .globl _clear_across
	    .globl _clear_lines
	    .globl _scroll_up
	    .globl _scroll_down
	    .globl _plot_char
	    .globl _cursor_disable
	    .globl _vdp_load_font

	    .globl _fontdata_6x8

;
; VDP routines are directly hooked into the vt layer
;
VDP_DIRECT	.equ	1
VDP_IRQ		.equ	0
VDP_ROP		.equ	1

;
;	The loops as they stand are good for up to about 8MHz. On a Z180
;	not only do they run 1 clock faster but we are running at a bit over
;	18MHz.
;

; We know we are using a TMS9918/28/29A so our timings worst case is
; 150 cycles at this speed. Our loop worst case is 25 cycles on Z80 (23 on
; Z180) so we need to burn a whole 127 cycles each access with no wait
; states. As we run with 2 I/O wait states we get to only wait 125 ...
.macro VDP_DELAY
		call twiddle_thumbs
.endm
;
;	Fetch case.
;
.macro VDP_DELAY2
		call twiddle_thumbs
.endm

twiddle_thumbs:			; Burn 125 clocks including the call return
		; We spend 27 getting here and going back
		ex (sp),ix	; 19
		ex (sp),ix	; 38
		ex (sp),ix	; 57
		ex (sp),ix	; 76
		ex (sp),ix	; 85
		ex (sp),ix	; 94
		nop		; 98
		ret

	    .include "../dev/vdp1.s"

	    .area _COMMONMEM

_vdpport:   .word 0x2899	; port 0x99, 40 byte count in fastest load
