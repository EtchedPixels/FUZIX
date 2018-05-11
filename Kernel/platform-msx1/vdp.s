            .module vdp

            .include "kernel.def"
            .include "../kernel.def"



	    .area _COMMONMEM

	    .globl _cursor_on
	    .globl _cursor_off
	    .globl _clear_across
	    .globl _clear_lines
	    .globl _scroll_up
	    .globl _scroll_down
	    .globl _plot_char
	    .globl _cursor_disable

;
; VDP routines are directly hooked into the vt layer
;
VDP_DIRECT	.equ	1

	    .include "../dev/vdp1.s"
;
;	FIXME: should use vdpport, but right now vdpport is in data not
;	common space.
;
	    .area _COMMONMEM

platform_interrupt_all:
	    ld c, #0x99
	    in a, (c)
_cursor_disable:
	    ret
