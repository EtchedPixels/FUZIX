;;;
;;;  Low-Level Video Routines
;;;
;;;
	.module	videoll


	;; exported
	.globl	_memset
	.globl	_memcpy
	.globl _video_read
	.globl _video_write
	.globl _video_cmd
	.globl _putq
	.globl _getq

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
	lda	#0x3f		; mmu page 10 for queue structs
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
	lda	#0x3f		; mmu page 10 for queue structs
	sta	0xffa9		;
	ldb	,x
	lda	#1		; restore kernel map
	sta	0xffa9		;
	puls	cc,pc		; restore interrupts, return
