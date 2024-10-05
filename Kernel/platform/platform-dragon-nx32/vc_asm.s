;
; low-level routines for VC handling
;

	.module	vc_asm

	.globl _set_vid_mode
	.globl _set_vc_mode

	include "kernel.def"

	.area .video

_set_vc_mode:
	ldx #$ffc6
	sta -4,x	; reset V1 0xffc2 ; set resolution
	sta -2,x	; reset V2 0xffc4

	; video base 0x1C00 = 0x200 * 0b1110
	sta   b,x	; set/reset bit F0 for b = 0 or 1
	sta 4+1,x	; set F2
	sta 6+1,x	; set F3

	lda $ff22
	anda #$07
	sta $ff22	; set PIA for VDG
	rts

_set_vid_mode:
	; video base 0x0400 = 0x200 * 0b0010
	sta $ffc6+0 ; reset F0
	sta $ffca+0 ; reset F2
	sta $ffcc+0 ; reset F3
	jmp _vid256x192	; set resolution

