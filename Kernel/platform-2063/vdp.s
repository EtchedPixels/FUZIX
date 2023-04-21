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


; TMS setup
VDP_IRQ		.equ	0
VDP_DIRECT	.equ	1

; 10MHz. TODO - do the maths but this should be roughly right
.macro VDP_DELAY
	nop
	nop
	nop
.endm
.macro VDP_DELAY2
	nop
	nop
	nop
.endm

	    .include "../dev/vdp1.s"
