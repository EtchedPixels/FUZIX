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

;
; VDP routines are directly hooked into the vt layer
;
VDP_DIRECT	.equ	1
VDP_IRQ		.equ	1
VDP_ROP		.equ	1

;
;	On an MSX at 3.5Mhz our loop worst case is 26 clocks so for
;	graphics one we need a nop
;
.macro VDP_DELAY
	    nop
.endm
.macro VDP_DELAY2
	    nop
.endm
	    .include "../../dev/vdp1.s"

	    .area _COMMONMEM

plt_interrupt_all:
	    ret

_vdpport:   .word 0x2899	; port 0x99, 40 byte count in fastest load
