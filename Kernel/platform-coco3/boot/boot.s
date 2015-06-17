;;;
;;;  A Fuzix booter for the CoCo3
;;;

;;; This bootloader works from a DECB-like evironment, It loads
;;; FUZIX.BIN from a DECB disk and plops it in memory, starting
;;; at physical address 0x0000.
	org	$2600		; where am I loaded.

frame	.dw	0		; on entry frame pointer
npage	.db	0		; next page no.
pos	.dw	0		; buffer pos in memory
nampre	fcn	/"FUZIX.BIN/	; " image to load

	;; And the Kick-off
start
	sts	frame
	lda	#'F		; print F
	jsr	0xa282		;
	;; open kernel image file
	ldb	#'I		; input mode
	jsr	open
	lbcs	abort
	lda	#'U		; print "U"
	jsr	$a282
	;; Move to task one
	ldx	#$ffa0
	ldu	#$ffa8
	ldd	,x++		; copy mmu regs
	std	,u++
	ldd	,x++
	std	,u++
	ldd	,x++
	std	,u++
	ldd	,x++
	std	,u++
	ldb	#1
	stb	$ff91		; set mmu to task 1
	;; Load BIN file
	;;
	ldb	$6f		; get current file no.
	pshs	b		; save on stack
	ldb	#1
	stb	$6f		; switch in/out routine to disk file #1
c@	jsr	$a176		; get a byte in A
	cmpa	#$ff		; compare A to ff
	beq	post		; jump to post able handling
	;; preamble
	jsr	getw		; D = length address
	tfr	d,y		; U = length
	jsr	getw		; D = load address
	jsr	setload		; set load address
d@	jsr	$a176		; A = byte
	jsr	putb		; put into kernel memory
	leay	-1,y		; decrement U
	bne	d@		; loop
	bra	c@		; try next byte in stream
	;; postable
post	jsr	getw		; get zero's
	cmpd	#0		; test D
	lbne	abort		; abort if not zero
	jsr	getw		; get exec address
	pshs	d		; save on stack
	jsr	close		; close DECB file
	;; report load
	clr	$6f		; set stream to console
	lda	#'L
	jsr	$A282
	ldd	#0		; Address = 0
	jsr	setload
	ldy	#bounce_end-bounce
	ldx	#bounce
e@	lda	,x+
	jsr	putb
	leay	-1,y
	bne	e@
	lda	,s+		; get jmp address high byte
	jsr	putb		; put at end of bounce routine
	lda	,s+
	jsr	putb		; put at end of bounce routine
	lda	#'O
	jsr	$a282		; report load
	;; find command line in input buffer
	ldx	#$2dc		; X = start of line
f@	lda	,x+		; get a byte
	cmpa	#$83		; is a colon token?
	bne	f@
	ldd	#$88		; set destination of command line
	jsr	setload		;
g@	lda	,x+		; get one byte
	jsr	putb		; put it in memory
	tsta
	bne	g@		; repeat if not done
	;; map in kernel block 0
	orcc	#$50		; turn off interrupts
	clr	$ffa8
	jmp	$0		; and jump to bounce routine

;;; This routine is copied down to kernel map 0
bounce
	;; map in rest of kernel
	ldx	#$ffa9
	lda	#1
a@	sta	,x+
	inca
	cmpa	#8
	bne	a@
	.db	$7e		; jump
bounce_end


;;; Gets next word from file
;;;    takes: nothing
;;;    returns: D = next word
getw
	jsr	$a176		; A = high byte
	tfr	a,b		; B = high byte
	jsr	$a176		; A = low byte
	exg	a,b		; flip D = next word
	rts


;;; Sets load address
;;;   takes: D = Address
;;;   returns: X = cpu mapped address
;;;   mods: D
setload
	pshs	d
	;; find block number
	lsra
	lsra
	lsra
	lsra
	lsra			; A= blk no
	sta	$ffaa		; put in mmu
	puls	d
	anda	#$1f		; D = offset
	addd	#$4000		; mmu offset
	std	pos		; store
	rts

;;; puts a byte into kernel memory, increments position
;;;   takes: A = byte to store
;;;   returns: nothing
;;;   mods:
putb	pshs	d,x
a@	ldx	pos
	cmpx	#$6000		; too far ?
	beq	inc@
	sta	,x+
	stx	pos
	puls	d,x,pc
inc@	ldx	#$4000		; inc pos
	stx	pos
	inc	$ffaa
	bra	a@

;;; Abort!
abort
	lds	frame
	rts


;;; Open a file
;;;   takes: B=ascii mode (I,O,D)
;;;   returns: C set on error
open	pshs	b		; save mode
	;; move local filename into BASIC's vars
	ldx	$a6
	pshs	x
	ldx	#nampre		; pointer to our local filename
	stx	$a6		; set CHARAD
	jsr	$c935		; have BASIC set up DNAMBF for us
	puls	x
	stx	$a6
	;; end of string copy - get directory info
f@	jsr	$c68c		; search directory, U=ram directory image
	tst	$973		; found?
	bne	g@		; yes - then return
	;; not found
	ldb	,s		; get mode
	cmpb	#'I		; is mode I?
	beq	err@		; yes then error!
	ldd	#$00ff		; basic/ascii
	bra	h@
	;; copy directory stuff
g@	ldd	,u		; D = type/ascii
	ldb	#$ff		; force ascii
h@	std	$957
	ldd	#$100		; record length
	std	$976
	lda	,s		; A = Mode (I,O,D)
	ldb	#1		; B = FCB #1
	jsr	$c48d		; open file
out@	clra			; clear C
	puls	b,pc		; return
err@	coma			; set C
	puls	b,pc		; return



;;; Close file buffer
close
	ldb	#1		; set file number to 1
	stb	$6f
	jsr	$a42d		; close file
	clr	$6f		; set dev to screen
	rts



	end	start
