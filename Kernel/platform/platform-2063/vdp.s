            .module vdp

            .include "kernel.def"
            .include "../../cpu-z80/kernel-z80.def"

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
VDP_ROP		.equ	1

;
;	We need 2us between the last control write and a data read. We might
;	be running at 10MHz or faster so use long delays
;
;	These will only work on the TMS9918A text mode but fail on the
;	later chips. There is no 9938/58 for the 2063 however
;
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

; The 2063 doesn't waitstate the I/O so back to back outs go funny
.macro VDP_DELAY3
	nop
.endm

	    .include "../../dev/vdp1.s"
