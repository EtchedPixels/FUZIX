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

	    .include "../dev/vdp1.s"

	    .area _COMMONMEM

platform_interrupt_all:
	    ret

_vdpport:   .word 0x2899	; port 0x99, 40 byte count in fastest load
