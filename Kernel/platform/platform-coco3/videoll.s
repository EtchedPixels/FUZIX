;;;
;;;  Low-Level Video Routines
;;;
;;;
	.module	videoll


	;; exported
	.globl _memset
	.globl _memcpy
	.globl _memcmp
	.globl _video_read
	.globl _video_write
	.globl _video_cmd
	.globl _putq
	.globl _getq
	.globl _cursor_off
	.globl _cursor_on
	.globl _cursor_disable
	.globl _plot_char
	.globl _clear_lines
	.globl _clear_across
	.globl _scroll_up
	.globl _scroll_down

	.globl _twidth
	.globl _theight

	include "kernel.def"
	include "../../cpu-6809/kernel09.def"
	
	.area .video

VIDEO_BASE  equ	 $4000

	
;;;   void *memset(void *d, int c, size_t sz)
_memset:
	pshs	x,y
	ldb	7,s
	ldy	8,s
a@	stb	,x+
	leay	-1,y
	bne	a@
	puls	x,y,pc



;;;   void *memcpy(void *d, const void *s, size_t sz)
_memcpy:
	pshs	x,y,u
	ldu	8,s
	ldy	10,s
a@	ldb	,u+
	stb	,x+
	leay	-1,y
	bne	a@
	puls	x,y,u,pc

;;;	int memcmp(const void *s1, const void *s2, size_t sz)
_memcmp:
	pshs	x,y,u
	ldu	8,s
	ldy	10,s
a@	ldb	,u+
	cmpb	,x+
	bcs	retneg
	bne	retpos
	leay	-1,y
	bne	a@
	ldd	#0
	puls	x,y,u,pc
retneg:
	ldd	#-1
	puls	x,y,u,pc
retpos:
	ldd	#1
	puls	x,y,u,pc

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

;;; void video_cmd( char *rle_data );
_video_cmd:
	pshs 	u
	bsr 	vidptr		; u now points to the screen
nextline:
	pshs 	u		; save it for the next line
nextop:
	ldb 	,x+		; op code, 0 = end of line
	beq 	endline
oploop:
	lda 	,u		; do one screen byte
	anda 	,x
	eora 	1,x
	sta 	,u+
	decb
	bne 	oploop		; keep going for run
	leax 	2,x
	bra 	nextop		; next triplet
endline:
	puls 	u		; get position back
	leau 	32,u		; down one scan line
	ldb 	,x+		; get next op - 0,0 means end and done
	bne 	oploop
	puls 	u,pc


;;; Calculate Screen pointer from y,x,h,w
;;;   takes: X = ptr to box
;;;   returns: X = X + 4
;;;   returns: U = screen ptr
;;;   modifies: D
vidptr:
	ldd	,x++
	lda	#32
	mul
	addd	,x++
	addd	#VIDEO_BASE
	tfr	d,u
	rts


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


;;;
;;; Low Level Queueing Routine
;;;


;;; Put a character into tty queue
;;; void putq( uint8_t *ptr, uint8_t c )
;;; takes: B = character, X = ptr to buffer (in queue-space)
;;; modifies: A
_putq
	pshs	cc		; save interrupt state
	orcc	#0x50		; stop interrupt till we restore map
	lda	#0x0C		; mmu page 0C for queue structs
	sta	0xffa9		;
	stb	,x		; store the character
	lda	#1		; restore kernel map
	sta	0xffa9		;
	puls	cc,pc		; restore interrupts, return

;;; Gets a character from tty queue
;;; uint8_t getq( uint8_t *ptr )
;;; takes: X = ptr to buffer (in queue-space)
;;; returns: B = retrieved character
;;; modifies: nothing
_getq
	pshs	cc		; save interrupt state
	orcc	#0x50		; stop interrupt till we restore map
	lda	#0x0C		; mmu page 0C for queue structs
	sta	0xffa9		;
	ldb	,x
	lda	#1		; restore kernel map
	sta	0xffa9		;
	puls	cc,pc		; restore interrupts, return

;
;	Console video
;
_cursor_off:
	ldx	_curtty
	ldx	2,x		; cpos
	beq	nocursor
	pshs	cc
	orcc	#$10
	ldd	#0x0809
	std	$FFA9
	lda	1,x
	eora	#0x3F
	sta	1,x
	ldd	#0x0102
	std	$FFA9
	puls	cc,pc
nocursor:
_cursor_disable:

_cursor_on:
	pshs	y,cc
	lda	5,s
	ldx	_curtty
	bsr	txtaddr
	lda	1,y
	eora	#0x3F
	sta	1,y
	ldd	#0x0102
	std	$FFA9
	sty	2,x		; curtty isn't mapped until we unmap the video!
	puls	y,cc,pc

txtaddr:		; A = X B = Y preserve X
	ldy	_curtty
	ldy	,y		; Base
	leay	a,y	; lea is signed
	leay	a,y	; 80 cols will overflow if we shift first
	lda	_twidth
	mul		; Might be sensible to optimize some modes ?
	leay	d,y
	ldd	#0x0809
	std	0xFFA9
do_rts:
	rts

_plot_char:
	pshs	y,cc
	orcc	#$10
	lda	5,s
	bsr	txtaddr
	tfr	x,d
	cmpb	#'_'
	beq	unsc
	cmpb	#'^'
	beq	hat
plotit:
	lda	_curattr
	sta	1,y
	stb	,y
	ldd	#0x0102
	std	$FFA9
	puls	y,cc,pc
unsc:
	ldb	#0x7F
	bra	plotit
hat:
	ldb	#0x5E
	bra	plotit

_clear_lines:
	pshs	y,u,cc
	clra
	bsr	txtaddr
	lda	7,s
	ldb	_twidth
	mul
	tfr	d,x
	lda	#$20
	ldb	_curattr
	tfr	y,u
	cmpx	#0
	beq	donea
clt:
	std	,u++
	leax	-1,x
	bne	clt
donea:
	ldd	#0x0102
	std	$FFA9
	puls	y,u,cc,pc

_clear_across:
	cmpx #0
	beq do_rts
	pshs y,cc
	lda 5,s
	orcc #$10
	bsr txtaddr
	ldb _curattr
	lda #$20
clat:
	std ,y++
	leax -1,x
	bne clat
	ldd #0x0102
	std $FFA9
	puls y,cc,pc

_scroll_up:
	pshs	u,y,cc
	orcc	#$10
	ldx	_curtty
	ldx	,x
	tfr	x,u	; save screen base in U as source
	ldb	_twidth	; in bytes (can be > 128 so use abx)
	abx		; X now points to line below.
	lda	_theight
	deca		; one line less for the copy
	mul
	tfr	d,y	; words to copy for a whole screen minus a line
	ldd	#0x0809
	std	$FFA9
scrupt:
	ldd	,x++
	std	,u++
	ldd	,x++
	std	,u++
	ldd	,x++
	std	,u++
	ldd	,x++
	std	,u++
	leay	-4,y
	bne	scrupt
	ldd	#0x0102
	std	$FFA9
	puls	u,y,cc
	rts

_scroll_down:
	pshs	u,y,cc
	orcc	#$10
	ldx	_curtty
	ldx	,x		; base
	ldb	_twidth
	lda	_theight
	deca
	mul
	tfr	d,y	; count
	leax	d,x	; end of screen bar one line
	tfr	x,u	; save
	ldb	_twidth
	abx		; true end (must be abx as > 128 and unsigned)
	ldd	#0x0809
	std	$FFA9

scrdnt:
	ldd	,--x
	std	,--u
	ldd	,--x
	std	,--u
	ldd	,--x
	std	,--u
	ldd	,--x
	std	,--u
	leay	-4,y
	bne	scrdnt
	ldd	#0x0102
	std	$FFA9
	puls	u,y,cc
	rts

	.area .videodata
_twidth:
	.byte 160
_theight:
	.byte 24
