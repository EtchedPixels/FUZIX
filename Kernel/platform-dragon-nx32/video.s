	.module dragonvideo

	; Methods provided
	.globl _vid256x192
	.globl _plot_char
	.globl _scroll_up
	.globl _scroll_down
	.globl _clear_across
	.globl _clear_lines
	.globl _cursor_on
	.globl _cursor_off
	;
	; Imports
	;
	.globl _fontdata_8x8

	include "kernel.def"
	include "../kernel09.def"

	.area .text

;
;	Dragon video drivers
;
_vid256x192:
	sta $ffc0
	sta $ffc3
	sta $ffc5
	lda $ff22
	anda #$07
	ora #$f8
	sta $ff22
	rts

;
;	Compute the video base address
;	A = X, B = Y
;
vidaddr:
	ldy #VIDEO_BASE	; X is now the right column top line
	leay a,y
	clra
	rorb	; 3 right is as good as 5 left	
	rora
	rorb
	rora
	rorb
	rora	; D is now 32 * b
	leay d,y
	rts
;
;	plot_char(int8_t y, int8_t x, uint16_t c)
;
_plot_char:
	pshs y
	lda 4,s
	bsr vidaddr		; preserves X (holding the char)
	tfr x,d
	rolb			; multiply by 8
	rola
	rolb
	rola
	rolb
	rola
	tfr d,x
	leax _fontdata_8x8,x		; relative to font
	lda ,x+			; simple 8x8 renderer for now
	sta 0,y
	lda ,x+
	sta 32,y
	lda ,x+
	sta 64,y
	lda ,x+
	sta 96,y
	lda ,x+
	sta 128,y
	lda ,x+
	sta 160,y
	lda ,x+
	sta 192,y
	lda ,x+
	sta 224,y
	puls y,pc

;
;	void scroll_up(void)
;
_scroll_up
	pshs y
	ldy VIDEO_BASE
	leax 256,y
vscrolln:
	; Unrolled line by line copy
	ldd ,x++
	std ,y++
	ldd ,x++
	std ,y++
	ldd ,x++
	std ,y++
	ldd ,x++
	std ,y++
	ldd ,x++
	std ,y++
	ldd ,x++
	std ,y++
	ldd ,x++
	std ,y++
	ldd ,x++
	std ,y++
	ldd ,x++
	std ,y++
	ldd ,x++
	std ,y++
	ldd ,x++
	std ,y++
	ldd ,x++
	std ,y++
	ldd ,x++
	std ,y++
	ldd ,x++
	std ,y++
	ldd ,x++
	std ,y++
	ldd ,x++
	std ,y++
	cmpy video_endptr
	bne vscrolln
	puls y,pc

;
;	void scroll_down(void)
;
_scroll_down:
	pshs y
	ldy VIDEO_END
	leax -256,y
vscrolld:
	; Unrolled line by line loop
	ldd ,--x
	std ,--y
	ldd ,--x
	std ,--y
	ldd ,--x
	std ,--y
	ldd ,--x
	std ,--y
	ldd ,--x
	std ,--y
	ldd ,--x
	std ,--y
	ldd ,--x
	std ,--y
	ldd ,--x
	std ,--y
	ldd ,--x
	std ,--y
	ldd ,--x
	std ,--y
	ldd ,--x
	std ,--y
	ldd ,--x
	std ,--y
	ldd ,--x
	std ,--y
	ldd ,--x
	std ,--y
	ldd ,--x
	std ,--y
	ldd ,--x
	std ,--y
	cmpx video_startptr
	bne vscrolld
	puls y,pc

video_startptr:
	.dw	VIDEO_BASE
video_endptr:
	.dw	VIDEO_END

;
;	clear_across(int8_t y, int8_t x, uint16_t l)
;
_clear_across:
	pshs y
	lda 4,s		; x into A, B already has y
	jsr vidaddr	; Y now holds the address
	tfr x,d		; Shuffle so we are writng to X and the counter
	tfr y,x		; l is in d
clearnext:
	clr ,x
	clr 32,x
	clr 64,x
	clr 96,x
	clr 128,x
	clr 160,x
	clr 192,x
	clr 224,x
	leax 1,x
	deca
	bne clearnext
	puls y,pc
;
;	clear_lines(int8_t y, int8_t ct)
;
_clear_lines:
	pshs y
	clra			; b holds Y pos already
	jsr vidaddr		; y now holds ptr to line start
	tfr y,x
	clra
	clrb
wipel:
	std ,x++
	std ,x++
	std ,x++
	std ,x++
	std ,x++
	std ,x++
	std ,x++
	std ,x++
	std ,x++
	std ,x++
	std ,x++
	std ,x++
	std ,x++
	std ,x++
	std ,x++
	std ,x++
	dec 4,s			; count of lines
	bne wipel
	puls y,pc

_cursor_on
	pshs y
	lda  4,s
	jsr vidaddr
	tfr y,x
	puls y
	stx cursor_save
	; Fall through
_cursor_off
	ldx cursor_save
	com ,x
	com 32,x
	com 64,x
	com 96,x
	com 128,x
	com 160,x
	com 192,x
	com 224,x
	rts

	.area .data
cursor_save:
	.dw	0
