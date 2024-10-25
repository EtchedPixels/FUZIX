;
;        cpc vt primitives
;

        .module cpcvideo

        ; exported symbols
        .globl _plot_char
        .globl _scroll_down
        .globl _scroll_up
        .globl _cursor_on
        .globl _cursor_off
		.globl _cursor_disable
        .globl _clear_lines
        .globl _clear_across
        .globl _do_beep
		.globl _fontdata_8x8
		.globl _curattr
		.globl _vtattr

		.globl map_video
		.globl map_kernel

	; Build the video library as the only driver

CPCVID_ONLY	.equ	1
SCREENBASE	.equ 0x40

.macro VIDEO_MAP
	call map_video
.endm

.macro VIDEO_UNMAP
	call map_kernel
.endm

	.globl _fontdata_8x8

;_fontdata_8x8	.equ	0xF000		; routines except this
									; to point to space char
        .area _COMMONMEM

	.include "../../dev/cpc/video.s"

