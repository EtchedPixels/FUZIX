	.module dragonvideo

	; Methods provided
	.globl _vid256x192
	.globl _vidtxt
	.globl _plot_char
	.globl _scroll_up
	.globl _scroll_down
	.globl _clear_across
	.globl _clear_lines
	.globl _cursor_on
	.globl _cursor_off
	.globl _cursor_disable
	.globl _vtattr_notify

	.globl _video_read
	.globl _video_write
	.globl _video_cmd

	;
	; Imports
	;
	.globl _fontdata_8x8
	.globl _vidattr
	.globl _vid_h
	.globl _vid_b

	include "kernel.def"
	include "../kernel09.def"

;
;	TODO: could hide the text video functions and init in discard ?
;
	.area .text

_vidtxt:
	; COCO2 defaults to text at 0400-05FF move it to 0200
	rts
;
;	Dragon video drivers
;
;	SAM V2=1 V1=1 V0=-
;	6847 A/G=1 GM2=1 GM1=1 GM0=1
;
_vid256x192:
	sta $ffc0
	sta $ffc3
	sta $ffc5
	lda $ff22
	anda #$07
	ora #$f0
	sta $ff22
	sta vidmode	; any non zero value will do
	ldb #24
	stb _vid_h
	decb
	stb _vid_b
	rts

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
vidaddr:
	ldy #VIDEO_BASE
	exg a,b
	leay d,y		; 256 x Y + X
	rts
;
;	plot_char(int8_t y, int8_t x, uint16_t c)
;
_plot_char:
	pshs y
	lda 4,s
	tst vidmode
	bne plot_cg3
	bsr txtaddr
	tfr x,d
	cmpb #0x60
	bcs upper
	subb #0x20
upper:
	andb #0x3F
	stb ,y
	puls y,pc
plot_cg3:
	bsr vidaddr		; preserves X (holding the char)
	tfr x,d
	andb #$7F		; no high font bits
	subb #$20		; skip control symbols
	rolb			; multiply by 8
	rola
	rolb
	rola
	rolb
	rola
	tfr d,x
	leax _fontdata_8x8,x		; relative to font
	ldb _vtattr
	andb #0x3F		; drop the bits that don't affect our video
	beq plot_fast

	;
	;	General purpose plot with attributes, we only fastpath
	;	the simple case
	;
	clra
plot_loop:
	sta _vtrow
	ldb _vtattr
	cmpa #7		; Underline only applies on the bottom row
	beq ul_this
	andb #0xFD
ul_this:
	cmpa #3		; italic shift right for < 3
	blt ital_1
	andb #0xFB
	bra maskdone
ital_1:
	cmpa #5		; italic shift right for >= 5
	blt maskdone
	bitb #0x04
	bne maskdone
	orb #0x40		; spare bit borrow for bottom of italic
	andb #0xFB
maskdone:
	lda ,x+			; now throw the row away for a bit
	bitb #0x10
	bne notbold
	lsra
	ora -1,x		; shift and or to make it bold
notbold:
	bitb #0x04		; italic by shifting top and bottom
	beq notital1
	lsra
notital1:
	bitb #0x40
	beq notital2
	lsla
notital2:
	bitb #0x02
	beq notuline
	lda #0xff		; underline by setting bottom row
notuline:
	bitb #0x01		; inverse or not: we are really in inverse
	bne plot_inv		; by default so we complement except if
	coma			; inverted
plot_inv:
	bitb #0x20		; overstrike or plot ?
	bne overstrike
	sta ,y
	bra plotnext
overstrike:
	anda ,y
	sta ,y
plotnext:
	leay 32,y
	lda _vtrow
	inca
	cmpa #8
	bne plot_loop
	puls y,pc
;
;	Fast path for normal attributes
;
plot_fast:
	lda ,x+			; simple 8x8 renderer for now
	coma
	sta 0,y
	lda ,x+
	coma
	sta 32,y
	lda ,x+
	coma
	sta 64,y
	lda ,x+
	coma
	sta 96,y
	lda ,x+
	coma
	sta 128,y
	lda ,x+
	coma
	sta 160,y
	lda ,x+
	coma
	sta 192,y
	lda ,x+
	coma
	sta 224,y
	puls y,pc

;
;	void scroll_up(void)
;
_scroll_up:
	pshs y
	ldy #VIDEO_BASE
	tst vidmode
	bne scroll_up_cg3
scrup_t:
	ldx #VIDEO_BASE+0x20
scrup_tl:
	ldd ,x++
	std ,y++
	cmpx #VIDEO_BASE+512
	bne scrup_tl
	puls y,pc

scroll_up_cg3:
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
	cmpx video_endptr
	bne vscrolln
	puls y,pc

;
;	void scroll_down(void)
;
;	Never used in text mode
;
_scroll_down:
	pshs y
	ldy #VIDEO_END
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
;	Never used in text mode
;
_clear_across:
	pshs y
	lda 4,s		; x into A, B already has y
	tst vidmode
	beq clat
	jsr vidaddr	; Y now holds the address
	tfr x,d		; Shuffle so we are writng to X and the counter
	tfr y,x		; l is in d
	lda #$ff
clearnext:
	sta ,x
	sta 32,x
	sta 64,x
	sta 96,x
	sta 128,x
	sta 160,x
	sta 192,x
	sta 224,x
	leax 1,x
	decb
	bne clearnext
	puls y,pc
clat:
	jsr txtaddr
	ldb #$20
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
	tst vidmode
	bne clines_cg3
	clra
	jsr txtaddr
	lsl 4,s
	lsl 4,s
	lsl 4,s
	lsl 4,s			; x 16 cos dword
	ldd #$2020
	tfr y,x
clt:
	std ,x++
	dec 4,s		; 32 bytes per line max 16 lines in text
	bne clt
	puls y,pc

clines_cg3:
	clra			; b holds Y pos already
	jsr vidaddr		; y now holds ptr to line start
	tfr y,x
	ldd #$ffff
	lsl 4,s
	lsl 4,s
	lsl 4,s
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

_cursor_on:
	pshs y
	lda  4,s
	tst vidmode
	bne con_cg3
	jsr txtaddr
	sty cursor_save
	tfr y,x
	puls y
coff_txt:
	lda #0x40
	eora ,x
	sta ,x
	rts

con_cg3:
	jsr vidaddr
	tfr y,x
	puls y
	stx cursor_save
	; Fall through
_cursor_off:
	ldx cursor_save
	tst vidmode
	beq coff_txt
	ldb _vtattr
	bitb #0x80
	bne nocursor
	com ,x
	com 32,x
	com 64,x
	com 96,x
	com 128,x
	com 160,x
	com 192,x
	com 224,x
nocursor:
_vtattr_notify:
_cursor_disable:
	rts
;
;	These routines wortk in both 256x192x2 and 128x192x4 modes
;	because everything in the X plane is bytewide.
;
_video_write:
	clra			; clr C
	bra	tfr_cmd
_video_read:
	coma			; set C
	bra	tfr_cmd		; go

;;; This does the job of READ & WRITE
;;;   takes: C = direction 0=write, 1=read
;;;   takes: X = transfer buffer ptr + 2
tfr_cmd:
	pshs	u,y		; save regs
	orcc	#$10		; turn off interrupt - int might remap kernel
	ldd	#$80c0		; this is writing
	bcc	c@		; if carry clear then keep D write
	exg	a,b		; else flip D: now is reading
c@	sta	b@+1		; !!! self modify inner loop
	stb	b@+3		; !!!  
	bsr	vidptr		; U = screen addr
	tfr	x,y		; Y = ptr to Height, width
	leax	4,x		; X = pixel data
	;; outter loop: iterate over pixel rows
a@	lda	3,y		; count = width
	pshs	u		; save screen ptr
	;; inner loop: iterate over columns
	;; modify mod+1 and mod+3 to switch directions
b@	ldb	,x+		; get a byte from src
	stb	,u+		; save byte to dest
	deca			; bump counter
	bne	b@		; loop
	;; increment outer loop
	puls	u		; restore original screen ptr
	leau	32,u		; add byte span of screen (goto next line)
	dec	1,y		; bump row counter
	bne	a@		; loop
	puls	u,y,pc		; restore regs, return
	

;
;	Find the address we need on a pixel row basis
;
vidptr:
	ldu #VIDEO_BASE
	ldd ,x++		; Y into B
	lda #32
	mul
	leau d,u
	ldd ,x++		; X
	leau d,u
	rts

_video_cmd:
	pshs u
	bsr vidptr		; u now points to the screen
nextline:
	pshs u			; save it for the next line
nextop:
	ldb ,x+			; op code, 0 = end of line
	beq endline
oploop:
	lda ,u			; do one screen byte
	anda ,x
	eora 1,x
	sta ,u+
	decb
	bne oploop		; keep going for run
	leax 2,x
	bra nextop		; next triplet
endline:
	puls u			; get position back
	leau 32,u		; down one scan line
	ldb ,x+			; get next op - 0,0 means end and done
	bne oploop
	puls u,pc

	.area .data
cursor_save:
	.dw	0
_vtrow:
	.db	0
vidmode:
	.db	0
