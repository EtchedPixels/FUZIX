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
;
;	These are 38 cycles a pair so 72 cycles all in. That should be
;	good for 10MHz and a fair bit more. We could tune the delays down
;	a bit if we were sure nobody went over 10MHz.
;
.macro VDP_DELAY
	ex (sp),hl
	ex (sp),hl
	ex (sp),hl
	ex (sp),hl
.endm

.macro VDP_DELAY2
	ex (sp),hl
	ex (sp),hl
	ex (sp),hl
	ex (sp),hl
.endm

	    .include "../../dev/vdp1.s"
