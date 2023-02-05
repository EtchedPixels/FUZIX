;
;	Although the COCO2 fits the video simple model the generated code
;	isn't terribly pretty.
;
		.module video


	include "kernel.def"
	include "../kernel09.def"

;
;	TODO: could hide the text video functions and init in discard ?
;
	.area .text3

	.globl _plot_char
	.globl _scroll_up
	.globl _scroll_down
	.globl _clear_across
	.globl _clear_lines
	.globl _cursor_on
	.globl _cursor_off
	.globl _cursor_disable
	.globl _vtattr_notify

;
;	Video base for text
;
txtaddr:
	ldy #VIDEO_BASE
	leay a,y		; X value
	lslb
	lslb
	lslb			; x 8 so keep unsigned
	leay b,y		; and add four times
	leay b,y
	leay b,y
	leay b,y
	rts
;
;	Compute the video base address
;	A = X, B = Y
;
;
;	plot_char(int8_t y, int8_t x, uint16_t c)
;
_plot_char:
	pshs y
	lda 4,s
	bsr txtaddr
	tfr x,d
	tst _lower
	beq map_cases
	; We want to map
;	; 20-3F	to 60-7F		40-5F	^20	60-7F
	; 40-5F to 40-5F		60-7F	^20	40-5F
	; 60-7F to 00-1F		80-9F	^20	A0-BF
	addb #$20
	bmi low
	eorb #$20
	bra plotit
low:
	andb #$1f
	bra plotit
map_cases:
	; A to Z we just mask with 0x3F so we get inverse
	cmpb #'A'
	bcs notuc
	cmpb #'Z'+1
	bcs mask_only
	; a to z we mask with 1f and add 0x40 to get normal
notuc:	cmpb #'a'
	bcs notlc
	cmpb #'z'+1
	bcs fixlc
	; lower punctuation, numbers etc  we just mask
notlc:	cmpb #$60
	bcs maskinv
	; upper punctuation we mask and invert
	andb #$1f
	bra plotit
fixlc:
	andb #$1f
maskinv:
	andb #$3f
	addb #$40
	bra plotit
mask_only:
	andb #$3F
plotit:
	stb ,y
	puls y,pc
;
;	void scroll_up(void)
;
_scroll_up:
	pshs y
	ldy #VIDEO_BASE
	ldx #VIDEO_BASE+0x20
scrup_tl:
	ldd ,x++
	std ,y++
	cmpx #VIDEO_BASE+512
	bne scrup_tl
	puls y,pc
;
;	void scroll_down(void)
;
_scroll_down:
	pshs y
	ldy #VIDEO_BASE+0x200
	ldx #VIDEO_BASE+0X1E0
scrdn_tl:
	ldd ,--x
	std ,--y
	cmpx #VIDEO_BASE
	bne scrdn_tl
	puls y,pc

;
;	clear_across(int8_t y, int8_t x, uint16_t l)
;
;	Never used in text mode
;
_clear_across:
	pshs y
	lda 4,s		; x into A, B already has y
	jsr txtaddr
	ldb _spaces
clatl:
	stb ,y+
	dec 4,s
	bne clatl
	puls y,pc
;
;	clear_lines(int8_t y, int8_t ct)
;
_clear_lines:
	pshs y
	clra
	jsr txtaddr
	lsl 4,s
	lsl 4,s
	lsl 4,s
	lsl 4,s			; x 16 cos dword
	ldd _spaces
	tfr y,x
clt:
	std ,x++
	dec 4,s		; 32 bytes per line max 16 lines in text
	bne clt
	puls y,pc

_cursor_on:
	pshs y
	lda  4,s
	jsr txtaddr
	sty cursor_save
	tfr y,x
	puls y
coff_txt:
	lda #0x40
	eora ,x
	sta ,x
	rts

_cursor_off:
	ldx cursor_save
	bra coff_txt
_vtattr_notify:
_cursor_disable:
	rts

	.area .data
cursor_save:
	.dw	0
_spaces
	.dw	0x6060
