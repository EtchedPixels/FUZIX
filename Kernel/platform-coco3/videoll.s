;;;
;;;  Low-Level Video Routines
;;;
;;;
	.module	videoll


	.globl	_memset
	.globl	_memcpy
*	.globl _vid256x192
*	.globl _plot_char
*	.globl _scroll_up
*	.globl _scroll_down
*	.globl _clear_across
*	.globl _clear_lines
*	.globl _cursor_on
*	.globl _cursor_off

	.globl _video_read
	.globl _video_write
	.globl _video_cmd

	include "kernel.def"
	include "../kernel09.def"
	
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




vidptr:
	ldu 	#VIDEO_BASE
	ldd 	,x++		; Y into B
	lda 	#32
	mul
	leau 	d,u
	ldd 	,x++		; X
	leau 	d,u
	rts


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
