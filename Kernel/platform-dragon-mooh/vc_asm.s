;
; low-level routines for VC handling
;

	.module	vc_asm

	.globl _set_vid_mode
	.globl _set_vc_mode
	.globl _vc_clear
	.globl _vc_write_char
	.globl _vc_read_char
	.globl _vc_memset
	.globl _vc_scroll_up
	.globl _vc_scroll_down

	include "kernel.def"

	.area .video

_set_vc_mode:
	ldx #$ffc6
	sta -4,x	; reset V1 0xffc2 ; set resolution
	sta -2,x	; reset V2 0xffc4

	; video base 0x0400 = 0x200 * 0b0010
	sta   b,x	; set/reset bit F0 for b = 0 or 1
	sta 2+1,x	; set F1
	sta 4+0,x 	; reset F2

	lda $ff22
	anda #$07
	sta $ff22	; set PIA for VDG
	rts

_set_vid_mode:
	; video base 0x0800 = 0x200 * 0b0100
	sta $ffc6+0 ; reset F0
	sta $ffc8+0 ; reset F1
	sta $ffca+1 ; set F2
	jmp _vid256x192	; set resolution

_vc_clear:
	tfr b,a
	asla
	clrb
	ldx #VC_BASE+512
	leax d,x
	pshs x
	leax -512,x
	jsr _map_video
	ldd #$2020
cllp	std ,x++
	cmpx ,s
	blo cllp
	leas 2,s
	jmp _unmap_video

_vc_write_char:
	jsr _map_video
	stb ,x
	jmp _unmap_video

_vc_read_char:
	jsr _map_video
	ldb ,x
	jmp _unmap_video

vtbase	lda _curtty
	deca
	deca
	asla
	clrb
	ldx #VC_BASE
	leax d,x
	leay 512,x
	rts

_vc_scroll_up:
	pshs y,x	; x: make space for end address
	jsr vtbase
	sty ,s
	leay 32,x
	jsr _map_video
scup	ldd ,y++
	std ,x++
	cmpx ,s
	blo scup
	jsr _unmap_video
	puls y,x,pc

_vc_scroll_down:
	pshs y,x
	jsr vtbase
	stx ,s
	leax -32,y
	jsr _map_video
scdn	ldd ,--x
	std ,--y
	cmpx ,s
	bhi scdn
	jsr _unmap_video
	puls y,x,pc

_vc_memset:
	pshs y
	ldy 4,s
	jsr _map_video
vcms	stb ,x+
	leay -1,y
	bne vcms
	jsr _unmap_video
	puls y,pc

