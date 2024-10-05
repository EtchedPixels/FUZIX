#include "kernel.def"
#include "../../cpu-z80u/kernel-z80.def"

	.common

	.export _cursor_on
	.export _cursor_off
	.export _clear_across
	.export _clear_lines
	.export _scroll_up
	.export _scroll_down
	.export _plot_char
	.export _cursor_disable
	.export _vdp_load_font

	.export _fontdata_6x8

;
; VDP routines are directly hooked into the vt layer
;
#define VDP_DIRECT
/* NMI not IRQ */
#undef VDP_IRQ
/* Can't use due to thunked code */
#undef VDP_ROP
;
;	On an MSX at 3.5Mhz our loop worst case is 26 clocks so for
;	graphics one we need a nop
;
#define	VDP_DELAY	nop
#define VDP_DELAY2	nop

#include "../../dev/vdp1u.S"

	.common

plt_interrupt_all:
	ret

_vdpport:
	.word 0x28A1	; port 0xA1, 40 byte count in fastest load
